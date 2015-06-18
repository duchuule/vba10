#pragma once
#include <collection.h>
#include <ppltasks.h>

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace Platform::Collections;
using namespace concurrency;



namespace VBA10
{
	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class ROMDBEntry sealed
	{
	public:
		ROMDBEntry(int locationtype, Platform::String^ displayname, Platform::String^ filename, Platform::String^ filepath);

		ROMDBEntry(int locationtype, Platform::String^ displayname, Platform::String^ filename, Platform::String^ filepath,
			DateTime lastplayed, int autosaveindex, Platform::String^ snapshoturi);

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

	private:
		int _id;
		int _locationType;
		Platform::String^ _displayName;
		Platform::String^ _fileName;
		Platform::String^ _filePath;
		Windows::Foundation::DateTime _lastPlayed;
		int _autoSaveIndex;
		Platform::String^ _snapshotUri;


	};

}