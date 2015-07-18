#pragma once
#include <windows.h>
#include <concrt.h>
#include "Statement.h"

namespace VBA10
{
	namespace SQLiteWinRT
	{
		using Windows::Foundation::Uri;
		using Windows::Foundation::IAsyncOperation;
		using Windows::Foundation::IAsyncAction;
		using Platform::String;
		using namespace Windows::Storage;

		public enum class SqliteOpenMode
		{
			Default = 0, // interpreted as OpenOrCreateReadWrite
			OpenRead = SQLITE_OPEN_READONLY,
			OpenReadWrite = SQLITE_OPEN_READWRITE,
			OpenOrCreateReadWrite = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
		};

		public ref class Database sealed
		{
		public:
			static IAsyncOperation<Database^>^ FromApplicationUriAsync(Uri^ path);

			static SqliteReturnCode GetSqliteErrorCode(int hr) { return SqliteReturnCode(HRESULT_CODE(hr)); }

			Database(StorageFile^ file);
			Database(StorageFolder^ folder, String^ name);

			virtual ~Database();

			property String^ Path { String^ get() { return m_path; } }

			IAsyncAction^ OpenAsync();
			IAsyncAction^ OpenAsync(SqliteOpenMode openMode);
			IAsyncOperation<Statement^>^ PrepareStatementAsync(String^ cmd);
			IAsyncAction^ ExecuteStatementAsync(String^ cmd);

			int64 GetLastInsertedRowId();

		internal:
			property sqlite3* RawDatabasePtr { sqlite3* get() const { return m_database; } }

		private:
			sqlite3* m_database;
			LONG volatile m_opened;
			String^ m_path;
			String^ m_filename;
		};
	}
}