#include "ROMDatabase.h"
#include "ppltasks_extra.h";

#include <collection.h>

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::AccessCache;
using namespace Windows::UI::Xaml;

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
				"LASTPLAYED         INT  NOT NULL,"\
				"LASTSAVEINDEX    INT  NOT NULL, "\
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

			//load rom entry
			return LoadSnapshotImage();
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


	task<void> ROMDatabase::AddAsync(ROMDBEntry^ entry)
	{
		return create_task([this, entry]
		{
			
			

			//prepare statement to add to rom table
			Platform::String^ cmd = "INSERT INTO ROMTABLE (LOCATIONTYPE, DISPLAYNAME, FILENAME, FILEPATH, LASTPLAYED, LASTSAVEINDEX, AUTOSAVEINDEX, SNAPSHOTURI) VALUES (";
			cmd += entry->LocationType + ",";
			cmd += "'" + entry->DisplayName + "',";
			cmd += "'" + entry->FileName + "',";
			cmd += "'" + entry->FilePath + "',";
			cmd += entry->LastPlayed.UniversalTime + ",";
			cmd += entry->LastSaveIndex + ",";
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
							DateTime{ statement->GetIntAt(4) }, statement->GetIntAt(5), statement->GetIntAt(6), statement->GetTextAt(7)
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



	task<void> ROMDatabase::LoadSnapshotImage()
	{
		vector<task<void>> tasks;

		for (int i= 0; i < _allROMDBEntries->Size; i++)
		{
			auto entry = _allROMDBEntries->GetAt(i);

			
			tasks.emplace_back(create_task([entry]
			{
				if (entry->LocationType == 0) //app storage
				{
					

					return create_task([entry]()->StorageFolder^
					{
						return ApplicationData::Current->LocalFolder;
					});
				}
				else // external storage
				{
					//find the folder in futureaccesslist
					//create token from file path
					Platform::String ^ptoken = entry->FilePath;
					wstring token(ptoken->Begin(), ptoken->End());

					//get rid of the file name, keep only the folder path
					size_t found = token.find_last_of(L"/\\");
					token = token.substr(0, found);

					replace(token.begin(), token.end(), ':', '_');
					replace(token.begin(), token.end(), '/', '_');
					replace(token.begin(), token.end(), '\\', '_');
					ptoken = ref new Platform::String(token.c_str());

					return create_task([entry, ptoken]
					{
						return StorageApplicationPermissions::FutureAccessList->GetFolderAsync(ptoken);
					});
				}
			}).then([entry](StorageFolder^ folder)
			{

				entry->Folder = folder;

				//get the snapshot file
				return folder->GetFileAsync(entry->SnapshotUri);

			}).then([entry](StorageFile ^file)
			{
				//open file
				return file->OpenAsync(FileAccessMode::Read);
			}).then([entry](IRandomAccessStream^ stream)
			{
				//load bitmap image for snapshot
				entry->Snapshot = ref new BitmapImage();
				return entry->Snapshot->SetSourceAsync(stream);
			}));
		}


		// When all tasks finish, create a continuation task that observes any exceptions that occurred. 
		return when_all(begin(tasks), end(tasks)).then([tasks](task<void> t)
		{
			try
			{
				t.get();
			}
			catch (COMException^ e)
			{
				// We'll handle the specific errors below.
			}

		});
	}


	task<void> ROMDatabase::UpdateAsync(ROMDBEntry^ entry)
	{
		return create_task([this, entry]
		{

			//prepare statement to update entry
			Platform::String^ cmd = "UPDATE ROMTABLE SET DISPLAYNAME = "; 
			cmd += "'" + entry->DisplayName + "',";
			cmd += " LASTPLAYED = " + entry->LastPlayed.UniversalTime + ",";
			cmd += " LASTSAVEINDEX = " + entry->LastSaveIndex + ",";
			cmd += " AUTOSAVEINDEX = " + entry->AutoSaveIndex ;
			cmd += " WHERE FILEPATH = '" + entry->FilePath + "';";

#if _DEBUG
			Platform::String ^message = cmd;
			wstring wstr(message->Begin(), message->End());
			OutputDebugStringW(wstr.c_str());
#endif
			return db->ExecuteStatementAsync(cmd);
		});
	}




}