#pragma once

#define DEFAULT_SNAPSHOT	L"Assets/no_snapshot.png"
#define EXPORT_FOLDER		L"VBA10 Export"

using namespace Windows::Storage;

namespace VBA10
{
	public delegate void ROMSelectedDelegate(StorageFile ^file, StorageFolder ^folder);

	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class StorageFolderModel sealed
	{
	private:
	public:
		property StorageFolder ^Folder;

		property Platform::String ^Name
		{
			Platform::String ^get()
			{
				return this->Folder->DisplayName;
			}
		}

		property Platform::String ^Path
		{
			Platform::String ^get()
			{
				return this->Folder->Path;
			}
		}

		StorageFolderModel(StorageFolder ^folder);
	};

	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class StorageFileModel sealed
	{
	private:
	public:
		property StorageFile ^File;
		property StorageFolder ^Folder;

		property Platform::String ^Name
		{
			Platform::String ^get()
			{
				return this->File->Name;
			}
		}

		property Platform::String ^Path
		{
			Platform::String ^get()
			{
				return this->Folder->Path;
			}
		}

		StorageFileModel(StorageFile ^file, StorageFolder ^folder);


	};

	public enum  class OneDriveItemType: int
	{
		File,
		Folder,
		ROM,
		SRAM,
		Savestate,
		Zip,
		Rar,
		SevenZip
	};


	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class OneDriveFileItem sealed : Windows::UI::Xaml::Data::INotifyPropertyChanged
	{
	public:
		
		property Platform::String^ Name;
		property OneDriveItemType Type;
		property bool Downloading
		{
			bool get()
			{
				return _downloading;
			}
			void set(bool value)
			{
				_downloading = value;

				OnPropertyChanged("Downloading");
			}
		}
		//public Stream Stream{ get; set; } //the Stream corresponding to this item
		property Platform::String^ OneDriveID;
		property Platform::String^ ParentID;
		property Platform::String^ OneDriveLink;
		property int FolderChildrenCount;
		property Windows::Storage::StorageFile^ File;  //to store the pointer to the file when downloading
		virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler^ PropertyChanged;
	protected:
		void OnPropertyChanged(Platform::String^ propertyName);

	private: 
		bool _downloading;

	};

}