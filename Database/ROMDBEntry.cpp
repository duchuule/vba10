#include "Database\ROMDBEntry.h"


#define DEFAULT_SNAPSHOT  "Assets/no_snapshot.png";

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace Platform::Collections;

namespace VBA10
{
	ROMDBEntry::ROMDBEntry(int locationtype, Platform::String^ displayname, Platform::String^ filename, Platform::String^ filepath)
		:_locationType(locationtype), _displayName(displayname), _fileName(filename), _filePath(filepath)
	{
		_lastPlayed = DateTime{ 0 };
		_autoSaveIndex = 0;
		_snapshotUri = DEFAULT_SNAPSHOT;
		
	}

	ROMDBEntry::ROMDBEntry(int locationtype, Platform::String^ displayname, Platform::String^ filename, Platform::String^ filepath,
		DateTime lastplayed, int autosaveindex, Platform::String^ snapshoturi)
		:_locationType(locationtype), _displayName(displayname), _fileName(filename), _filePath(filepath),
		_lastPlayed(lastplayed), _autoSaveIndex(autosaveindex), _snapshotUri(snapshoturi)
	{

	}



}