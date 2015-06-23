#include "pch.h"
#include "Converter.h"
#include "Definitions.h"
#include "stringhelper.h"
#include "EmulatorFileHandler.h"
#include "DirectXPage.xaml.h"

using namespace Windows::UI::Xaml::Media::Imaging;
using namespace VBA10;
using namespace Concurrency;
using namespace Platform;
using namespace std;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::AccessCache;

IsoImageConverter::IsoImageConverter()
{
}

IsoImageConverter::~IsoImageConverter()
{
}

Object^ IsoImageConverter::Convert(Object^ value, TypeName targetType, Object^ parameter, String^ language)
{
	try
	{
		BitmapImage^ bitmap = ref new BitmapImage();
		String^ path = (String^)value;
		if (path->Equals(DEFAULT_SNAPSHOT))
			return path;

		if (!path->IsEmpty())
		{
			//using (var isoStore = IsolatedStorageFile.GetUserStoreForApplication())
			//{
			//	using (IsolatedStorageFileStream fs = isoStore.OpenFile(path, System.IO.FileMode.Open, System.IO.FileAccess.Read))
			//	{
			//		bitmap.SetSource(fs);
			//	}
			//}
			//create_task([this, bitmap, path] 
			//{
			//	if (IsROMLoaded()) //create snapshot
			//		return DirectXPage::Current->TakeSnapshot();
			//	else
			//		return create_task([] {});
			//		
			//}).then([this, bitmap, path] {
				LoadImageFromPath(bitmap, path);
			//});
			

			
		}
		return bitmap;
	}
	catch (Exception^ ex)
	{
		return nullptr;
	}

}

task<void> IsoImageConverter::LoadImageFromPath(BitmapImage^ bitmap, String^ filepath)
{

		Platform::String ^file_path = filepath;

		wstring wfilepath(file_path->Begin(), file_path->End());

		wstring folderpath;
		wstring filename;
		wstring filenamenoext;
		wstring ext;
		splitFilePath(wfilepath, folderpath, filename, filenamenoext, ext);

		replace(folderpath.begin(), folderpath.end(), ':', '_');
		replace(folderpath.begin(), folderpath.end(), '/', '_');
		replace(folderpath.begin(), folderpath.end(), '\\', '_');
		Platform::String^ ptoken = ref new Platform::String(folderpath.c_str());
		Platform::String^ pfilename = ref new Platform::String(filename.c_str());

		return create_task(StorageApplicationPermissions::FutureAccessList->GetFolderAsync(ptoken))
			.then([bitmap, pfilename](StorageFolder^ folder)
		{
			return folder->GetFileAsync(pfilename);
		}).then([bitmap](StorageFile^ file)
		{
			return file->OpenAsync(FileAccessMode::Read);
		}).then([bitmap](IRandomAccessStream^ stream)
		{
			return bitmap->SetSourceAsync(stream);
		});



}

Object^ IsoImageConverter::ConvertBack(Object^ value, TypeName targetType, Object^ parameter, String^ language)
{
	return nullptr; //doing one-way binding so this is not required.
}

