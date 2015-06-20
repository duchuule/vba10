#include "Statement.h"
#include "Database.h"
#include "NativeBuffer.h"
#include <ppltasks.h>
#include <memory>
#include <functional>
#include <collection.h>
#include <robuffer.h>


using namespace SQLiteWinRT;
using namespace Platform;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;
using namespace concurrency;
using namespace Microsoft::WRL;


std::unique_ptr<char []> SQLiteWinRT::PlatformStringToCharArray(Platform::String^ string)
{
	//copy from msdn, need this to get correct buffer size
	https://social.msdn.microsoft.com/forums/windowsapps/en-US/ce2ecbe8-f009-41c0-bbf4-e6f20eca9eb5/winrt-to-win32-string-converstions
	auto requiredBufferSize = WideCharToMultiByte(
		CP_UTF8,
		WC_ERR_INVALID_CHARS,
		string->Data(),
		static_cast<int>(string->Length()),
		nullptr,
		0,
		nullptr,
		nullptr
		);

	requiredBufferSize++;

	auto wideData = string->Data();

	std::unique_ptr<char []> ansi(new char[requiredBufferSize]);
	if (0 == WideCharToMultiByte(CP_UTF8, 0, wideData, -1, ansi.get(), requiredBufferSize, NULL, NULL))
		throw ref new FailureException(L"Can't convert string to UTF8");

	return ansi;
}

void SQLiteWinRT::WasteTimeToSimulateSlowOperation()
{
	auto handle = GetCurrentThread();
	WaitForSingleObjectEx(handle, WASTE_TIME_MS, FALSE);
}

Streams::IBuffer ^SQLiteWinRT::CreateNativeBuffer(LPVOID lpBuffer, DWORD nNumberOfBytes)
{
	Microsoft::WRL::ComPtr<NativeBuffer> nativeBuffer;
	Microsoft::WRL::Details::MakeAndInitialize<NativeBuffer>(&nativeBuffer, (byte *) lpBuffer, nNumberOfBytes);
	auto iinspectable = (IInspectable *)reinterpret_cast<IInspectable *>(nativeBuffer.Get());
	Streams::IBuffer ^buffer = reinterpret_cast<Streams::IBuffer ^>(iinspectable);

	return buffer;
}

Statement::Statement(Database^ database, Platform::String^ cmd)
{
	auto rc = (SqliteReturnCode) sqlite3_prepare16_v2(database->RawDatabasePtr, cmd->Data(), -1, &m_statement, 0);

	if (rc == SqliteReturnCode::Error)
		throw ref new Platform::InvalidArgumentException("SQL Error");

	if (rc != SqliteReturnCode::Ok)
		throw ref new Platform::COMException(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, rc), L"Can't prepare the statement");

	m_noMoreRows = false;
}

void Statement::ThrowIfStepFailed(SqliteReturnCode rc)
{
	switch (rc)
	{
	case SqliteReturnCode::Ok:
	case SqliteReturnCode::Done:
	case SqliteReturnCode::MoreRows:
		break;

	case SqliteReturnCode::ConstraintViolation:
		throw ref new Platform::Exception(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, rc), L"Constraint violation");

	case SqliteReturnCode::ReadOnlyConstraint:
		throw ref new Platform::Exception(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, rc), L"Database is read-only");

	case SqliteReturnCode::OutOfMemory:
		throw ref new Platform::OutOfMemoryException();

	default:
		throw ref new Platform::COMException(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, rc), L"Can't execute the statement");
	}
}

void Statement::ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw Exception::CreateException(hr);
	}
}


IAsyncOperation<bool>^ Statement::StepAsync()
{
	CheckForMoreRows();

	return create_async([this]()
	{
		auto result = Step();

		switch (result)
		{
		case SqliteReturnCode::Done:
			m_noMoreRows = true;
			break;

		case SqliteReturnCode::MoreRows:
			break;

		default:
			ThrowIfStepFailed(result);
			break;
		}

		if (!m_noMoreRows && m_columnsEnabled)
		{
			if (m_columns == nullptr)
				m_columns = ref new Platform::Collections::Map<String^, String^>();
			else
				m_columns->Clear();

			for (auto i = 0; i < GetColumnCount(); i++)
			{
				m_columns->Insert(GetColumnName(i), GetTextAt(i));
			}
		}

		return !m_noMoreRows;
	});
}

SqliteReturnCode Statement::Step(void)
{
	SimulateSlowOperation();
	auto result = (SqliteReturnCode) sqlite3_step(m_statement);
	return result;
}


int Statement::GetParameterIndex(Platform::String^ name)
{
	auto result = GetParameterIndexNoThrow(name);
	if (result <= 0)
		throw ref new Platform::InvalidArgumentException(L"Supplied 'name' doesn't exist");

	return result;
}

int Statement::GetParameterIndexNoThrow(Platform::String^ name)
{
	// parameter index must always be UTF8.
	auto utf8name = PlatformStringToCharArray(name);
	int result = sqlite3_bind_parameter_index(m_statement, utf8name.get());
	return result;
}

Statement::~Statement()
{
	sqlite3_finalize(m_statement);
}

void Statement::CheckForMoreRows()
{
	if (m_noMoreRows)
		throw ref new Platform::FailureException("No more rows");
}

Platform::String^ Statement::GetTextAt(int index)
{
	CheckForMoreRows();
	return ref new Platform::String((wchar_t*) sqlite3_column_text16(m_statement, index));
}

int Statement::GetIntAt(int index)
{
	CheckForMoreRows();
	return sqlite3_column_int(m_statement, index);
}

int64 Statement::GetInt64At(int index)
{
	CheckForMoreRows();
	return sqlite3_column_int64(m_statement, index);
}

double Statement::GetDoubleAt(int index)
{
	CheckForMoreRows();
	return sqlite3_column_double(m_statement, index);
}

IBuffer^ Statement::GetBlobAt(int index)
{
	CheckForMoreRows();
	const void *bytes = sqlite3_column_blob(m_statement, index);
	Streams::IBuffer ^buffer = CreateNativeBuffer((LPVOID) bytes, sqlite3_column_bytes(m_statement, index));
	return buffer;
}

void ThrowIfBindingFailed(SqliteReturnCode rc)
{
	if (rc == SqliteReturnCode::IndexOutOfRange)
		throw ref new Platform::OutOfBoundsException(L"Index out of range");

	if (rc == SqliteReturnCode::TypeMismatch)
		throw ref new Platform::InvalidArgumentException(L"Type mis-match");

	if (rc != SqliteReturnCode::Ok)
		throw ref new Platform::COMException(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, rc), L"Couldn't bind value");
}

void Statement::BindTextParameterAt(int index, Platform::String^ param)
{
	auto rc = (SqliteReturnCode) sqlite3_bind_text16(m_statement, index, param->Data(), -1, SQLITE_STATIC);
	ThrowIfBindingFailed(rc);
}

void Statement::BindIntParameterAt(int index, int param)
{
	auto rc = (SqliteReturnCode) sqlite3_bind_int(m_statement, index, param);
	ThrowIfBindingFailed(rc);
}

void Statement::BindInt64ParameterAt(int index, int64 param)
{
	auto rc = (SqliteReturnCode) sqlite3_bind_int64(m_statement, index, param);
	ThrowIfBindingFailed(rc);
}

void Statement::BindDoubleParameterAt(int index, double param)
{
	auto rc = (SqliteReturnCode) sqlite3_bind_double(m_statement, index, param);
	ThrowIfBindingFailed(rc);
}

void Statement::BindNullParameterAt(int index)
{
	auto rc = (SqliteReturnCode) sqlite3_bind_null(m_statement, index);
	ThrowIfBindingFailed(rc);
}

void Statement::BindBlobParameterAt(int index, IBuffer^ param)
{
	if (param == nullptr)
		throw ref new Platform::NullReferenceException(L"buffer cannot be null");

	// To access the buffer, we need to get an interface to IBufferByteAccess
	Object^ obj = param;
	ComPtr<IInspectable> insp(reinterpret_cast<IInspectable*>(obj));
	// Query IBufferByteAccess
	ComPtr<IBufferByteAccess> bufferByteAccess = NULL;
	ThrowIfFailed(insp.As(&bufferByteAccess));

	byte* bytes = nullptr;
	ThrowIfFailed(bufferByteAccess->Buffer(&bytes));

	auto rc = (SqliteReturnCode) sqlite3_bind_blob(m_statement, index, bytes, param->Length, SQLITE_STATIC);
	ThrowIfBindingFailed(rc);
}

void Statement::BindIntParameterWithName(Platform::String^ name, int param)
{
	BindPrimitiveParameter(m_statement, name, param, sqlite3_bind_int);
}

void Statement::BindInt64ParameterWithName(Platform::String^ name, int64 param)
{
	BindPrimitiveParameter(m_statement, name, param, sqlite3_bind_int64);
}

void Statement::BindDoubleParameterWithName(Platform::String^ name, double param)
{
	BindPrimitiveParameter(m_statement, name, param, sqlite3_bind_double);
}

void Statement::BindTextParameterWithName(Platform::String^ name, Platform::String^ param)
{
	int index = GetParameterIndexNoThrow(name);
	if (index < 0)
		throw ref new Platform::InvalidArgumentException("Column 'name' doesn't exist");

	auto rc = (SqliteReturnCode) sqlite3_bind_text16(m_statement, index, param->Data(), -1, SQLITE_STATIC);
	ThrowIfBindingFailed(rc);
}

void Statement::BindNullParameterWithName(Platform::String^ name)
{
	int index = GetParameterIndexNoThrow(name);
	if (index < 0)
		throw ref new Platform::InvalidArgumentException("Column 'name' doesn't exist");

	auto rc = (SqliteReturnCode) sqlite3_bind_null(m_statement, index);
	ThrowIfBindingFailed(rc);
}

void Statement::BindBlobParameterWithName(Platform::String^ name, IBuffer^ param)
{
	int index = GetParameterIndexNoThrow(name);
	if (index < 0)
		throw ref new Platform::InvalidArgumentException("Column 'name' doesn't exist");

	BindBlobParameterAt(index, param);
}

template<typename T>
void Statement::BindPrimitiveParameter(sqlite3_stmt* statement, Platform::String^ name, T value, int func(sqlite3_stmt*, int, T))
{
	int index = GetParameterIndexNoThrow(name);
	if (index < 0)
		throw ref new Platform::InvalidArgumentException("Column 'name' doesn't exist");

	auto rc = (SqliteReturnCode) func(statement, index, value);
	ThrowIfBindingFailed(rc);
}

int Statement::GetColumnCount()
{
	return sqlite3_column_count(m_statement);
}

String^ Statement::GetColumnName(int index)
{
	return ref new String((wchar_t*) sqlite3_column_name16(m_statement, index));
}

ColumnType Statement::GetColumnType(int index)
{
	return (ColumnType) sqlite3_column_type(m_statement, index);
}

void Statement::ClearBindings()
{
	auto rc = (SqliteReturnCode) sqlite3_clear_bindings(m_statement);
	if (rc != SqliteReturnCode::Ok)
		throw ref new Platform::COMException(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, rc), L"Couldn't clear bindings");
}

void Statement::Reset()
{
	auto rc = (SqliteReturnCode) sqlite3_reset(m_statement);
	if (rc != SqliteReturnCode::Ok)
		throw ref new Platform::COMException(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, rc), L"Couldn't reset connection");

	m_noMoreRows = false;
}

IMapView<String^, String^>^ Statement::Columns::get()
{
	if (m_columns == nullptr)
		return nullptr;

	return m_columns->GetView();
}
