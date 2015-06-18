#pragma once
#include <sqlite3.h>
#include <windows.h>
#include <concrt.h>
#include <memory>
#include <collection.h>


// Use this to help uncover assumptions in your code about how fast
// a database call will be.
#ifdef _DEBUG
//#define SIMUlATE_SLOW_OPERATIONS
#endif

#ifdef SIMULATE_SLOW_OPERATIONS
#define SimulateSlowOperation SQLiteWinRTPhone::WasteTimeToSimulateSlowOperation
#else
#define SimulateSlowOperation void
#endif

namespace SQLiteWinRT
{
	using Platform::String;

	const DWORD WASTE_TIME_MS = 300;
	void WasteTimeToSimulateSlowOperation();

	std::unique_ptr<char []> PlatformStringToCharArray(String^ string);
	Windows::Storage::Streams::IBuffer ^CreateNativeBuffer(LPVOID lpBuffer, DWORD nNumberOfBytes);

	public enum class SqliteReturnCode
	{
		Ok = SQLITE_OK,
			Error = SQLITE_ERROR,
			InternalError = SQLITE_INTERNAL,
			AccessDenied = SQLITE_PERM,
			AbortRequested = SQLITE_ABORT,
			DatabaseInUse = SQLITE_BUSY,
			TableLocked = SQLITE_LOCKED,
			OutOfMemory = SQLITE_NOMEM,
			ReadOnlyConstraint = SQLITE_READONLY,
			Interrupted = SQLITE_INTERRUPT,
			IOError = SQLITE_IOERR,
			DatabaseCorrupt = SQLITE_CORRUPT,
			UnknownOpcode = SQLITE_NOTFOUND,
			DatabaseFull = SQLITE_FULL,
			DatabaseOpenError = SQLITE_CANTOPEN,
			LockingProtocolError = SQLITE_PROTOCOL,
			DatabaseEmpty = SQLITE_EMPTY,
			SchemaChanged = SQLITE_SCHEMA,
			DataTooLarge = SQLITE_TOOBIG,
			ConstraintViolation = SQLITE_CONSTRAINT,
			TypeMismatch = SQLITE_MISMATCH,
			InvalidOperation = SQLITE_MISUSE,
			NotSupported = SQLITE_NOLFS,
			AuthorizationDenied = SQLITE_AUTH,
			BadDatabaseFormat = SQLITE_FORMAT,
			IndexOutOfRange = SQLITE_RANGE,
			InvalidFileFormat = SQLITE_NOTADB,
			LogNotification = SQLITE_NOTICE,
			LogWarning = SQLITE_WARNING,
			MoreRows = SQLITE_ROW,
			Done = SQLITE_DONE,
	};

	public enum class ColumnType
	{
		Text = SQLITE_TEXT,
			Integer = SQLITE_INTEGER,
			Double = SQLITE_FLOAT,
			Null = SQLITE_NULL,
			Blob = SQLITE_BLOB,
	};

	ref class Database;

	using Windows::Foundation::IAsyncOperation;
	using Windows::Foundation::Collections::IMap;
	using Windows::Foundation::Collections::IMapView;

	public ref class Statement sealed
	{

	private:
		sqlite3_stmt* m_statement;
		bool m_noMoreRows;
		bool m_columnsEnabled;
		Platform::Collections::Map<String^, String^>^ m_columns;

		SqliteReturnCode Step(void);
		int GetParameterIndexNoThrow(String^ name);
		int GetColumnCount();
		void CheckForMoreRows();

		template<typename T>
		void BindPrimitiveParameter(sqlite3_stmt* statement, String^ name, T value, int func(sqlite3_stmt*, int, T));

	internal:
		Statement(Database^ database, String^ cmd);
		static void ThrowIfStepFailed(SqliteReturnCode rc);
		static void ThrowIfFailed(HRESULT hr);

	public:
		property int ColumnCount { int get() { return GetColumnCount(); } };

		property IMapView<String^, String^>^ Columns { IMapView<String^, String^>^ get(); };
		void EnableColumnsProperty() { m_columnsEnabled = true; }

		IAsyncOperation<bool>^ StepAsync();

		String^ GetTextAt(int index);
		int GetIntAt(int index);
		int64 GetInt64At(int index);
		double GetDoubleAt(int index);
		Windows::Storage::Streams::IBuffer^ GetBlobAt(int index);

		void BindTextParameterAt(int index, String^ param);
		void BindIntParameterAt(int index, int param);
		void BindInt64ParameterAt(int index, int64 param);
		void BindDoubleParameterAt(int index, double param);
		void BindNullParameterAt(int index);
		void BindBlobParameterAt(int index, Windows::Storage::Streams::IBuffer^ buffer);

		void BindTextParameterWithName(String^ name, String^ param);
		void BindIntParameterWithName(String^ name, int param);
		void BindInt64ParameterWithName(String^ name, int64 param);
		void BindDoubleParameterWithName(String^ name, double param);
		void BindNullParameterWithName(String^ name);
		void BindBlobParameterWithName(String^ name, Windows::Storage::Streams::IBuffer^ buffer);

		int GetParameterIndex(String^ name);

		String^ GetColumnName(int index);
		ColumnType GetColumnType(int index);

		void ClearBindings();
		void Reset();

		virtual ~Statement();
	};
}
