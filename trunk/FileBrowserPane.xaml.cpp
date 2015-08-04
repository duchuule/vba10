//
// FileBrowserPane.xaml.cpp
// Implementation of the FileBrowserPane class
//

#include "pch.h"
#include "FileBrowserPane.xaml.h"
#include "App.xaml.h"

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
	root->Name = "Root";
	root->OneDriveID = "me/skydrive";
	root->Type = OneDriveItemType::Folder;
	root->ParentID = "";

	this->onedriveStack->GetAt(0)->Append(root);
	this->txtCurrentFolder->Text = root->Name;

	//get the content of the root
	this->txtLoading->Visibility = Windows::UI::Xaml::Visibility::Visible;
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
			wstring ext = type.substr(index + 1);

			a->Type = GetOneDriveItemType(ext);  //default, will change below
		}

		a->OneDriveID = ref new String(album[L"id"].as_string().c_str());
		a->ParentID = ref new String(album[L"parent_id"].as_string().c_str());
		a->FolderChildrenCount = album[L"count"].as_integer();
		a->OneDriveLink = ref new String(album[L"link"].as_string().c_str());

		this->fileVector->Append(a);
	}

	this->onedriveStack->Append(fileVector);
	this->FileListvs->Source = this->fileVector;
	this->fileList->SelectedItem = nullptr;
	this->txtLoading->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}


void FileBrowserPane::fileList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{

}


void FileBrowserPane::closeBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	(safe_cast<Popup ^>(this->Parent))->IsOpen = false;
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
