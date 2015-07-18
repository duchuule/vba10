#include "Database.h"
#include <ppltasks.h>

using namespace VBA10::SQLiteWinRT;
using namespace Platform;
using namespace concurrency;
using namespace Windows::Storage;
using namespace std;

//std::unique_ptr<char[]> PlatformStringToCharArray(Platform::String^ string)
//{
//  auto wideData = string->Data();
//  int bufferSize = string->Length() + 1;
//  std::unique_ptr<char[]> ansi(new char[bufferSize]);
//  if (0 == WideCharToMultiByte(CP_UTF8, 0, wideData, -1, ansi.get(), bufferSize, NULL, NULL))
//    throw ref new FailureException(L"Can't convert string to UTF8");
//
//  return ansi;
//}

IAsyncAction^ Database::OpenAsync()
{
  return OpenAsync(SqliteOpenMode::Default);
}

IAsyncAction^ Database::OpenAsync(SqliteOpenMode openMode)
{
  LONG result = InterlockedCompareExchange(&m_opened, 1, 0);
  if (result == 1)
    throw ref new Platform::FailureException(L"Database is already open (or is in the process of being opened)");

  return create_async([this, openMode]()
  {
    return create_task([this, openMode]()
    {
      SimulateSlowOperation();

      auto buffer(PlatformStringToCharArray(m_path));
      auto actualOpenMode = (openMode == SqliteOpenMode::Default) ? SqliteOpenMode::OpenOrCreateReadWrite : openMode;

      auto rc = (SqliteReturnCode)sqlite3_open_v2(buffer.get(), &m_database, (int)actualOpenMode, nullptr);

      switch(rc)
      {
      case SqliteReturnCode::Ok:
        break;

      case SqliteReturnCode::AccessDenied:
      case SqliteReturnCode::AuthorizationDenied:
        throw ref new Platform::AccessDeniedException(L"Access denied trying to open database");

      case SqliteReturnCode::BadDatabaseFormat:
      case SqliteReturnCode::DatabaseCorrupt:
      case SqliteReturnCode::DatabaseFull:
      case SqliteReturnCode::DatabaseEmpty:
      case SqliteReturnCode::DatabaseInUse:
      case SqliteReturnCode::DatabaseOpenError:
      case SqliteReturnCode::InvalidFileFormat:
        throw ref new Platform::Exception(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, rc), L"Database file is full / corrupt / empty / etc.");

      default:
        throw ref new Platform::Exception(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, rc), L"Unknonwn error opening file");
      }
    });
  });
}

IAsyncOperation<Statement^>^ Database::PrepareStatementAsync(String^ cmd)
{
  return create_async([this, cmd]() 
  { 
    SimulateSlowOperation(); 
    return ref new Statement(this, cmd); 
  } );
}

IAsyncAction^ Database::ExecuteStatementAsync(String^ cmd)
{
  return create_async([this, cmd]()
  {
    SimulateSlowOperation(); 

    auto utf8cmd = PlatformStringToCharArray(cmd);

    auto result = (SqliteReturnCode)sqlite3_exec(m_database, utf8cmd.get(), NULL, NULL, NULL);

    Statement::ThrowIfStepFailed(result);
  });
}

int64 Database::GetLastInsertedRowId() 
{ 
  return sqlite3_last_insert_rowid(m_database); 
}

IAsyncOperation<Database^>^ Database::FromApplicationUriAsync(Uri^ path)
{
  return create_async([path]()
  {
    try
    {
      return create_task(StorageFile::GetFileFromApplicationUriAsync(path))
        .then([](StorageFile^ file)
      {
        SimulateSlowOperation(); 
        return ref new Database(file);
      }
      );
    }
    catch (...)
    {
      throw ref new Platform::InvalidArgumentException("Path invalid");
    }
  });
}

Database::Database(StorageFile^ file) :
  m_path(file->Path)
{
}

Database::Database(StorageFolder^ folder, String^ name) :
  m_path(String::Concat(folder->Path, String::Concat(L"\\", name)))
{
}

Database::~Database()
{
  if (m_database != nullptr) 
  {
    sqlite3_close(m_database);
    m_database = nullptr;
  }
}
