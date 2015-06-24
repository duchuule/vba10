#pragma once
#include <collection.h>
#include <ppltasks.h>

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace Platform::Collections;
using namespace concurrency;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Storage;


namespace VBA10
{
	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class ROMDBEntry sealed
	{
	public:
		ROMDBEntry(int locationtype, Platform::String^ displayname, Platform::String^ filename, Platform::String^ filepath,
			Platform::String^ snapshoturi);

		ROMDBEntry(int locationtype, Platform::String^ displayname, Platform::String^ filename, Platform::String^ filepath,
			DateTime lastplayed, int lastsaveindex, int autosaveindex, Platform::String^ snapshoturi);

		//property int ID
		//{
		//	int get()
		//	{
		//		return _id;
		//	}
		//	void set(int value)
		//	{
		//		_id = value;
		//	}
		//}
		

		property int LocationType  //0: local folder, 1: external
		{
			int get()
			{
				return _locationType;
			}
			void set(int value)
			{
				_locationType = value;
			}
		}

		

		property Platform::String^ DisplayName
		{
			Platform::String^ get()
			{
				return _displayName;
			}
			void set(Platform::String^ value)
			{
				_displayName = value;
			}
		}

		property Platform::String^ FileName
		{
			Platform::String^ get()
			{
				return _fileName;
			}
			void set(Platform::String^ value)
			{
				_fileName = value;
			}
		}

		property Platform::String^ FilePath
		{
			Platform::String^ get()
			{
				return _filePath;
			}
			void set(Platform::String^ value)
			{
				_filePath = value;
			}
		}

		property Windows::Foundation::DateTime LastPlay
		{
			Windows::Foundation::DateTime get()
			{
				return _lastPlayed;
			}
			void set(Windows::Foundation::DateTime value)
			{
				_lastPlayed = value;
			}
		}

		property int LastSaveIndex
		{
			int get()
			{
				return _lastSaveIndex;
			}
			void set(int value)
			{
				_lastSaveIndex = value;
			}
		}

		property int AutoSaveIndex
		{
			int get()
			{
				return _autoSaveIndex;
			}
			void set(int value)
			{
				_autoSaveIndex = value;
			}
		}

		property Platform::String^ SnapshotUri
		{
			Platform::String^ get()
			{
				return _snapshotUri;
			}
			void set(Platform::String^ value)
			{
				_snapshotUri = value;
			}
		}

		//this is to hold temporary information about ROM folder when the app is running (for convenience)
		property StorageFolder^ Folder
		{
			StorageFolder^ get()
			{
				return _folder;
			}
			void set(StorageFolder^ value)
			{
				_folder = value;
			}
		}

		property StorageFile^ File
		{
			StorageFile^ get()
			{
				return _file;
			}
			void set(StorageFile^ value)
			{
				_file = value;
			}
		}

		//this is just to hold the snapshot image while the app is running
		property BitmapSource^ Snapshot
		{
			BitmapSource^ get()
			{
				return _snapshot;
			}
			void set(BitmapSource^ value)
			{
				_snapshot = value;
			}
		}

	private:
		int _id;
		int _locationType;
		Platform::String^ _displayName;
		Platform::String^ _fileName;
		Platform::String^ _filePath;
		Windows::Foundation::DateTime _lastPlayed;
		int _lastSaveIndex;
		int _autoSaveIndex;  //use to hold what save state to upload automatically
		Platform::String^ _snapshotUri;
		BitmapSource^ _snapshot;
		StorageFolder^ _folder;
		StorageFile^ _file;


	};

}