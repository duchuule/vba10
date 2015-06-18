#include "ROMDatabase.h"
#include "ppltasks_extra.h";

#include <collection.h>

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;

using namespace Concurrency;
using namespace Concurrency::extras;
using namespace std;


#define ROMDB_FILE_NAME "romdb.db"


//ROMDatabase^ ROMDatabase::singleton;

namespace VBA10
{

	ROMDatabase::ROMDatabase()
	{
	}


	task<void> ROMDatabase::Initialize(void)
	{

		return create_task(Windows::Storage::ApplicationData::Current->LocalFolder->CreateFileAsync(ROMDB_FILE_NAME, CreationCollisionOption::OpenIfExists))
			.then([this](StorageFile^ file)
		{
			db = ref new SQLiteWinRT::Database(file);

			//open database
			return db->OpenAsync(SQLiteWinRT::SqliteOpenMode::OpenReadWrite);
			
		}).then([this]
		{
			//create table if not exist
			return db->ExecuteStatementAsync("CREATE TABLE IF NOT EXISTS  ROMTABLE ( "\
				"LOCATIONTYPE           INT    NOT NULL,"\
				"DISPLAYNAME           TEXT    NOT NULL,"\
				"FILENAME            INT     NOT NULL,"\
				"FILEPATH            INT     NOT NULL,"\
				"LASTPLAY         INT  NOT NULL,"\
				"AUTOSAVEINDEX    INT  NOT NULL, "\
				"SNAPSHOTURI      TEXT NOT NULL );");
		}).then([this]
		{
			return db->PrepareStatementAsync("SELECT * FROM ROMTABLE;");
			
		}).then([this](SQLiteWinRT::Statement^ stmt)
		{
			statement = stmt;
			return RetrieveQuerry();

		}).then([this] (Vector<ROMDBEntry^>^ ret)
		{
			_allROMDBEntries = ret;

		}).then([](task<void> t)
		{
			return t.get();
		});




		//auto items = ref new Vector<String^>();


		//__await db->OpenAsync(SQLiteWinRT::SqliteOpenMode::OpenRead);
		//auto stmt = __await db->PrepareStatementAsync("SELECT rowid, CityName FROM Cities;");
		//while (__await stmt->StepAsync())
		//	items->Append(stmt->GetIntAt(0) + ": " + stmt->GetTextAt(1));
	}


	task<void> ROMDatabase::Add(ROMDBEntry^ entry)
	{
		return create_task([this, entry]
		{
			//first add this rom to the list 
			_allROMDBEntries->Append(entry);

			//prepare statement to add to rom table
			Platform::String^ cmd = "INSERT INTO ROMTABLE (LOCATIONTYPE, DISPLAYNAME, FILENAME, FILEPATH, LASTPLAY, AUTOSAVEINDEX, SNAPSHOTURI) VALUES (";
			cmd += entry->LocationType + ",";
			cmd += "'" + entry->DisplayName + "',";
			cmd += "'" + entry->FileName + "',";
			cmd += "'" + entry->FilePath + "',";
			cmd += entry->LastPlay.UniversalTime + ",";
			cmd += entry->AutoSaveIndex + ",";
			cmd += "'" + entry->SnapshotUri + "')";

#if _DEBUG
			Platform::String ^message = cmd;
			wstring wstr(message->Begin(), message->End());
			OutputDebugStringW(wstr.c_str());
#endif
			return db->ExecuteStatementAsync(cmd);
		});
	}


	//call this after prepare statement async
	task<Vector<ROMDBEntry^>^> ROMDatabase::RetrieveQuerry()
	{
		auto items = ref new Vector<ROMDBEntry^>();

		

		//now carry out the tasks, start with one StepAsync to see table has any data
		return create_task([this]
		{
			return statement->StepAsync();
		}).then([this, items](bool ret)
		{
			if (ret)
			{
				return create_iterative_task([this, items]
				{
					return create_task([this, items]
					{
						ROMDBEntry^ entry = ref new ROMDBEntry(
							statement->GetIntAt(0), statement->GetTextAt(1), statement->GetTextAt(2), statement->GetTextAt(3),
							DateTime{ statement->GetIntAt(4) }, statement->GetIntAt(5), statement->GetTextAt(6)
							);
						items->Append(entry);

						return statement->StepAsync();
					});
				});;
			}
			else
				return create_task([] {});
		}).then([items](task<void> t)
		{
			return items; //t.get();
		});



	}
}