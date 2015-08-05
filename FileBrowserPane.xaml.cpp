//
// FileBrowserPane.xaml.cpp
// Implementation of the FileBrowserPane class
//

#include "pch.h"
#include "FileBrowserPane.xaml.h"
#include "App.xaml.h"
#include "stringhelper.h"

using namespace VBA10;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Popups;
using namespace std;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

FileBrowserPane::FileBrowserPane()
{
	InitializeComponent();

	this->fileVector = ref new Platform::Collections::Vector<OneDriveFileItem^>();
	this->FileListvs->Source = this->fileVector;
	this->fileList->SelectedItem = nullptr;

	//load the skydrive list
	// Make an asynchronous call to OneDrive's REST interface to get the
	// user's album info, which is formatted as JSON objects, then parse
	// the JSON, create local objects and store them in the data model for this page.

	this->onedriveStack = ref new Vector<IVector<OneDriveFileItem^>^>();
	this->onedriveStack->Append(ref new Platform::Collections::Vector<OneDriveFileItem^>());

	//create the root item
	OneDriveFileItem^ root = ref new OneDriveFileItem();
	root->Name = "OneDrive";
	root->OneDriveID = "me/skydrive";
	root->Type = OneDriveItemType::Folder;
	root->ParentID = "";

	this->onedriveStack->GetAt(0)->Append(root);
	this->txtCurrentFolder->Text = root->Name;

	//get the content of the root
	this->txtLoading->Visibility = Windows::UI::Xaml::Visibility::Visible;
	this->loading = true;
	this->backBtn->IsEnabled = false;

	App::LiveClient->get(L"/me/skydrive/files")
		.then([this](web::json::value v)
	{
		client_GetCompleted(v);
		// Perform updates on the UI thread
		// to avoid the E_RPC_WRONG_THREAD exception.
	}, task_continuation_context::use_current())
		.then([](task<void> t)
	{
		try
		{
			// Handle exceptions in task chain.
			t.get();
		}

		//catch (const http_exception& e)
		//{
		//	wostringstream str;
		//	str << hex << e.error_code();
		//	dm->Status = L"Not connected. HTTP error code: " + ref new String(str.str().c_str());
		//}

		catch (const exception& e)
		{
			//wostringstream str;
			//str << e.what();
			//dm->Status = L"Not connected. Error: " + ref new String(str.str().c_str());
		}
	}, task_continuation_context::use_current());

}



void FileBrowserPane::client_GetCompleted(web::json::value v)
{

	this->fileVector = ref new Platform::Collections::Vector<OneDriveFileItem^>();
	//int test = v[L"data"].as_array().size();
	for (const auto& it : (v[L"data"]).as_array())
	{
		auto album = it;
		OneDriveFileItem^ a = ref new OneDriveFileItem();

		wstring name = album[L"name"].as_string();
		a->Name = ref new String(name.c_str());

		wstring type = album[L"type"].as_string();
		if (type == L"folder" || type == L"album")
			a->Type = OneDriveItemType::Folder;
		else
		{
			//get extension
			int index = name.find_last_of('.');
			wstring ext = name.substr(index + 1);

			a->Type = GetOneDriveItemType(ext);  //default, will change below
		}

		a->OneDriveID = ref new String(album[L"id"].as_string().c_str());
		a->ParentID = ref new String(album[L"parent_id"].as_string().c_str());
		if (a->Type == OneDriveItemType::Folder)
			a->FolderChildrenCount = album[L"count"].as_integer();
		a->OneDriveLink = ref new String(album[L"link"].as_string().c_str());

		this->fileVector->Append(a);
	}

	this->onedriveStack->Append(fileVector);
	this->FileListvs->Source = this->fileVector;
	this->fileList->SelectedItem = nullptr;
	this->txtLoading->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	if (this->onedriveStack->Size <= 2)
		this->backBtn->IsEnabled = false;
	else
		this->backBtn->IsEnabled = true;
	this->loading = false;
}


void FileBrowserPane::fileList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{

	if (this->loading)
		return;

	OneDriveFileItem ^ item = (OneDriveFileItem ^)(this->fileList->SelectedItem);
	if (item == nullptr)
		return;

	if (item->Type == OneDriveItemType::Folder)
	{

		this->FileListvs->Source = nullptr;
		this->txtCurrentFolder->Text = item->Name;


		//get the content of the folder
		this->txtLoading->Visibility = Windows::UI::Xaml::Visibility::Visible;
		this->loading = true;
		this->backBtn->IsEnabled = false;
		Platform::String ^id = item->OneDriveID;
		wstring wid(id->Begin(), id->End());

		App::LiveClient->get(wid +  L"/files")
			.then([this](web::json::value v)
		{
			client_GetCompleted(v);

		}, task_continuation_context::use_current())
			.then([](task<void> t)
		{
			try
			{
				// Handle exceptions in task chain.
				t.get();
			}

			catch (const exception& e)
			{
			}
		}, task_continuation_context::use_current());

	}
	else if (item->Type == OneDriveItemType::ROM)
	{
		if (item->Downloading)  //prevent double downloading of 1 file
			return;

		item->Downloading = true;
		//download file
		DownloadFile(item, CreationCollisionOption::GenerateUniqueName).then([this, item](size_t size)
		{
			//update rom dabatase
			//calculate snapshot name
			Platform::String ^file_path = item->File->Path;
			wstring wfilepath(file_path->Begin(), file_path->End());

			wstring folderpath;
			wstring filename;
			wstring filenamenoext;
			wstring ext;
			splitFilePath(wfilepath, folderpath, filename, filenamenoext, ext);

			wstring snapshotname = filenamenoext + L".jpg";
			Platform::String^ psnapshotname = ref new Platform::String(snapshotname.c_str());
			Platform::String^ pfilenamenoext = ref new Platform::String(filenamenoext.c_str());

			//create rom entry
			ROMDBEntry^ entry = ref new ROMDBEntry(0, pfilenamenoext, item->File->Name, ApplicationData::Current->LocalFolder->Path,
				"none", psnapshotname);

			entry->Folder = ApplicationData::Current->LocalFolder;

			App::ROMDB->AllROMDBEntries->Append(entry);

			create_task(App::ROMDB->AddAsync(entry)).then([entry] {
				//copy the default snapshot file over
				StorageFolder ^installDir = Windows::ApplicationModel::Package::Current->InstalledLocation;
				return installDir->GetFolderAsync("Assets/");

			}).then([entry, item](StorageFolder^ assetFolder)
			{
				return assetFolder->GetFileAsync("no_snapshot.png");
			}).then([entry, item](StorageFile ^file)
			{
				//copy snapshot file to would be location
				return file->CopyAsync(entry->Folder, entry->SnapshotUri, NameCollisionOption::ReplaceExisting);

			}).then([entry](StorageFile ^file)
			{
				//open file
				return file->OpenAsync(FileAccessMode::Read);
			}).then([entry, item](IRandomAccessStream^ stream)
			{
				//load bitmap image for snapshot
				entry->Snapshot = ref new BitmapImage();
				return entry->Snapshot->SetSourceAsync(stream);


			}).then([item](task<void> t)
			{
				try
				{
					t.get();
					// .get() didn't throw, so we succeeded, print out success message
					item->Downloading = false;
					MessageDialog ^dialog = ref new MessageDialog(item->File->Name + " was imported successfully.");
					dialog->ShowAsync();
				}
				catch (Platform::Exception ^ex)
				{
					MessageDialog ^dialog = ref new MessageDialog("Error: " + ex->Message);
					dialog->ShowAsync();
				}
			});
		});

		
	}
	else if (item->Type == OneDriveItemType::SRAM || item->Type == OneDriveItemType::Savestate)
	{
		if (item->Downloading)  //prevent double downloading of 1 file
			return;

		//get the save name without extension
		wstring name(item->Name->Begin(), item->Name->End());
		int index = name.find_last_of('.');
		//wstring ext = name.substr(index + 1);

		wstring filenamenoext = name.substr(0, index);


		Platform::String^ pfilenamenoext = ref new Platform::String(filenamenoext.c_str());

		ROMDBEntry^ entry = App::ROMDB->GetEntryFromName(pfilenamenoext);

		if (entry == nullptr)
		{
			MessageDialog ^dialog = ref new MessageDialog("Could not find a matching ROM name.", "Error");
			dialog->ShowAsync();
		}
		else
		{
			//start download the file
			item->Downloading = true;
			DownloadFile(item, CreationCollisionOption::ReplaceExisting).then([item, entry](size_t size)
			{
				item->Downloading = false;

				//if the file is ingame save then prevent autoloading
				if (item->Type == OneDriveItemType::SRAM)
					entry->AutoLoadLastState = false;

				MessageDialog ^dialog = ref new MessageDialog(item->File->Name + " was imported successfully.");
				dialog->ShowAsync();
			});
		}
	}
	else
	{
		MessageDialog ^dialog = ref new MessageDialog("This file type is not supported.");
		dialog->ShowAsync();
	}

}

task<size_t> FileBrowserPane::DownloadFile(OneDriveFileItem^ item, CreationCollisionOption collitionOption)
{
	return create_task(ApplicationData::Current->LocalFolder->CreateFileAsync(item->Name, collitionOption))
		.then([this, item] (StorageFile^ file)
	{
		item->File = file;
		return App::LiveClient->download(item->OneDriveID->Data(), file);
	}).then([](task<size_t> t) 
	{
		try
		{
			return t.get();
		}
		catch (Platform::Exception^ ex)
		{
			// We'll handle the specific errors below.
			MessageDialog ^dialog = ref new MessageDialog("Error: " + ex->Message);
			dialog->ShowAsync();

			size_t length = 0;
			return length;
		}

		
	});
}


void FileBrowserPane::closeBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//check to see if any thing is downloading
	for (unsigned int i = 0; i < this->fileVector->Size; i++)
	{
		OneDriveFileItem^ item = this->fileVector->GetAt(i);
		if (item->Downloading)
		{
			MessageDialog ^dialog = ref new MessageDialog("Please wait until all files have finished downloading.");
			dialog->ShowAsync();
			return;
		}
	}


	if (Frame->CanGoBack)
	{
		Frame->GoBack();
	}
}


void FileBrowserPane::OnNavigatingFrom(Windows::UI::Xaml::Navigation::NavigatingCancelEventArgs^ e)
{
	for (unsigned int i = 0; i < this->fileVector->Size; i++)
	{
		OneDriveFileItem^ item = this->fileVector->GetAt(i);
		if (item->Downloading)
		{
			MessageDialog ^dialog = ref new MessageDialog("Please wait until all files have finished downloading.");
			dialog->ShowAsync();
			e->Cancel = true;
			break;
		}
	}
	
}


OneDriveItemType FileBrowserPane::GetOneDriveItemType(wstring ext)
{
	if (ext == L"zip" || ext == L"zib")
	{
		return OneDriveItemType::Zip;
	}
	else if (ext == L"rar")
	{
		return OneDriveItemType::Rar;
	}
	else if (ext == L"7z")
	{
		return OneDriveItemType::SevenZip;
	}
	else if (ext == L"gb" || ext == L"gbc" || ext == L"gba")
	{
		return OneDriveItemType::ROM;
	}
	else if (ext == L"sgm")
	{
		return OneDriveItemType::Savestate;
	}
	else if (ext == L"sav")
	{
		return OneDriveItemType::SRAM;
	}
	return OneDriveItemType::File;
}


void FileBrowserPane::backBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//TODO: add handle while downloading

	if (this->onedriveStack->Size <= 2)
	{
		return;
	}

	//check to see if any thing is downloading
	for (unsigned int i = 0; i < this->fileVector->Size; i++)
	{
		OneDriveFileItem^ item = this->fileVector->GetAt(i);
		if (item->Downloading)
		{
			MessageDialog ^dialog = ref new MessageDialog("Please wait until all files have finished downloading.");
			dialog->ShowAsync();
			return;
		}
	}

	//remove last folder in stack
	this->onedriveStack->RemoveAtEnd();

	//set source for the upper folder
	this->loading = true;
	IVector<OneDriveFileItem^>^ currentFolder = this->onedriveStack->GetAt(this->onedriveStack->Size - 1);
	this->FileListvs->Source = currentFolder;
	this->fileList->SelectedItem = nullptr;

	//find parent name
	String^ parentName = "";

	if (this->onedriveStack->Size == 2) //special case
	{
		parentName = this->onedriveStack->GetAt(0)->GetAt(0)->Name;
		this->backBtn->IsEnabled = false;
	}
	else
	{
		IVector<OneDriveFileItem^>^ parentFolder = this->onedriveStack->GetAt(this->onedriveStack->Size - 2);
		OneDriveFileItem^ currentItem = currentFolder->GetAt(0);
		for (unsigned int i = 0; i <= parentFolder->Size - 1; i++)
		{
			OneDriveFileItem^ parentItem = parentFolder->GetAt(i);
			if (parentItem->OneDriveID == currentItem->ParentID)
			{
				parentName = parentItem->Name;
				break;
			}
		}
		this->backBtn->IsEnabled = true;
	}
	this->txtCurrentFolder->Text = parentName;

	this->loading = false;
}
