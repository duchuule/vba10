#pragma once


#include <collection.h>
#include <ppltasks.h>
#include "ROMDBEntry.h"
#include "Database.h"
#include "ROMDBEntry.h"




using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace Platform::Collections;
using namespace Concurrency;



namespace VBA10
{
	namespace WFC = Windows::Foundation::Collections;
	namespace PC  = Platform::Collections;

	ref class ROMDatabase sealed
	{
	public:

		ROMDatabase();
	internal:
		task<void> Initialize(void);
		task<void> Add(ROMDBEntry^ entry);
		task<Vector<ROMDBEntry^>^> RetrieveQuerry();
		property PC::Vector<ROMDBEntry^>^ AllROMDBEntries
		{
			PC::Vector<ROMDBEntry^>^ get()
			{
				return _allROMDBEntries;
			}
		}

	private:
		//static ROMDatabase^ singleton;

		SQLiteWinRT::Database^ db;
		SQLiteWinRT::Statement^ statement; //temporary pointer to store statement

		PC::Vector<ROMDBEntry^>^ _allROMDBEntries;
		
	};

		

}