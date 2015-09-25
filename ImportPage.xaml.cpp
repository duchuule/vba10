//
// ImportPage.xaml.cpp
// Implementation of the ImportPage class
//

#include "pch.h"
#include "ImportPage.xaml.h"
#include "Database\ROMDBEntry.h"
#include "Database\ROMDatabase.h"
#include "SelectFilesPane.xaml.h"
#include "App.xaml.h"
#include "FileBrowserPane.xaml.h"
#include "AdControl.xaml.h"

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
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage;
using namespace Windows::Storage::AccessCache;
using namespace Windows::Storage::Search;
using namespace Windows::Storage::Streams;
using namespace Concurrency;


// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

ImportPage::ImportPage()
{
	InitializeComponent();

	//create ad control
	if (App::HasAds)
	{
		AdControl^ adControl = ref new AdControl();
		LayoutRoot->Children->Append(adControl);
		adControl->SetValue(Grid::RowProperty, 2);
	}
}


void ImportPage::chooseFolderbtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto loader = Windows::ApplicationModel::Resources::ResourceLoader::GetForCurrentView();

	FolderPicker ^picker = ref new FolderPicker();

	picker->FileTypeFilter->Append(".gba");
	picker->FileTypeFilter->Append(".gbc");
	picker->FileTypeFilter->Append(".gb");
	//picker->FileTypeFilter->Append(".sav");
	//picker->FileTypeFilter->Append(".sgm");
	//picker->FileTypeFilter->Append(".zip");
	//picker->FileTypeFilter->Append(".rar");
	//picker->FileTypeFilter->Append(".7z");

	picker->ViewMode = PickerViewMode::List;
	picker->CommitButtonText = loader->GetString("SelectROMDirText");

	task<void> t = create_task(picker->PickSingleFolderAsync()).then([this, loader](StorageFolder ^folder)
	{
		if (folder)
		{
			//store folder
			//this->tmpfolder = folder;
			auto tmpfolder = make_shared<StorageFolder ^>(folder);

			//remove special char in path so that we can use path as token
			Platform::String ^ptoken = folder->Path;

			wstring token(ptoken->Begin(), ptoken->End());
			replace(token.begin(), token.end(), ':', '_');
			replace(token.begin(), token.end(), '/', '_');
			replace(token.begin(), token.end(), '\\', '_');
			ptoken = ref new Platform::String(token.c_str());

			//this->tmptoken = ptoken;
			auto tmptoken = make_shared<String^>(ptoken);
			

			//add folder to future accesslist
			if (!StorageApplicationPermissions::FutureAccessList->ContainsItem(ptoken))
				StorageApplicationPermissions::FutureAccessList->AddOrReplace(ptoken, folder);

			//get list of qualified file in folder
			Search::QueryOptions ^options = ref new Search::QueryOptions();
			options->FileTypeFilter->Append(".gba");
			options->FileTypeFilter->Append(".gbc");
			options->FileTypeFilter->Append(".gb");
			//TODO: add support for other file types
			return create_task(folder->CreateFileQueryWithOptions(options)->GetFilesAsync())
				.then([this, tmpfolder, tmptoken, loader](IVectorView<StorageFile ^> ^files)
			{

				//open panel to let user select file
				Popup ^statePopup = ref new Popup();
				statePopup->IsLightDismissEnabled = true;

				Vector<Platform::String ^> ^fileNames = ref new Vector<Platform::String ^>();
				for (int i = 0; i < files->Size; i++)
					fileNames->Append(files->GetAt(i)->Name);

				SelectFilesPane ^pane = ref new SelectFilesPane(fileNames, loader->GetString("SelectFileImportText"));
				statePopup->Child = pane;
				pane->Width = titleBar->ActualWidth;//statePopup->Width;
				pane->MaxHeight = Window::Current->Bounds.Height - 48; //statePopup->MaxHeight;

				pane->FilesSelectedCallback = ref new FilesSelectedDelegate([this, tmpfolder, tmptoken, files, loader](IVector<int>^ selectedIndices)
				{

					vector<task<void>> tasks;

					for (int i = 0; i < selectedIndices->Size; i++)
					{
						StorageFile^ file = files->GetAt(selectedIndices->GetAt(i));

						//calculate snapshot name
						Platform::String ^file_path = file->Path;
						wstring wfilepath(file_path->Begin(), file_path->End());

						wstring folderpath;
						wstring filename;
						wstring filenamenoext;
						wstring ext;
						splitFilePath(wfilepath, folderpath, filename, filenamenoext, ext);

						wstring snapshotname = filenamenoext + L".jpg";
						Platform::String^ psnapshotname = ref new Platform::String(snapshotname.c_str());
						Platform::String^ pfilenamenoext = ref new Platform::String(filenamenoext.c_str());


						//create rom entry or just return the original
						bool exist = false;
						ROMDBEntry^ entry = nullptr;
						for (int j = 0; j < App::ROMDB->AllROMDBEntries->Size; j++)
						{
							entry = App::ROMDB->AllROMDBEntries->GetAt(j);
							if (entry->FileName == file->Name && entry->Token == *tmptoken)
							{
								exist = true;
								break;
							}
						}

						if (!exist)
						{
							entry = ref new ROMDBEntry(1, pfilenamenoext, file->Name, (*tmpfolder)->Path,
								*tmptoken, psnapshotname);

							entry->Folder = *tmpfolder;

							App::ROMDB->AllROMDBEntries->Append(entry);



							tasks.emplace_back(
								create_task(App::ROMDB->AddAsync(entry)).then([entry] {
								//copy the default snapshot file over
								StorageFolder ^installDir = Windows::ApplicationModel::Package::Current->InstalledLocation;
								return installDir->GetFolderAsync("Assets/");

							}).then([entry](StorageFolder^ assetFolder)
							{
								return assetFolder->GetFileAsync("no_snapshot.png");
							}).then([entry](StorageFile ^file)
							{
								//copy snapshot file to would be location
								return file->CopyAsync(entry->Folder, entry->SnapshotUri, NameCollisionOption::ReplaceExisting);

							}).then([entry](StorageFile ^file)
							{
								//open file
								return file->OpenAsync(FileAccessMode::Read);
							}).then([entry](IRandomAccessStream^ stream)
							{
								//load bitmap image for snapshot
								entry->Snapshot = ref new BitmapImage();
								return entry->Snapshot->SetSourceAsync(stream);


							}).then([](task<void> t)
							{
								try
								{
									t.get();

								}
								catch (...)
								{
								}
							})

								);  //end of tasks.emplace_back
						}
					}

					when_all(begin(tasks), end(tasks)).then([this, loader](task<void> t)
					{
						try
						{
							t.get();
							this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([loader]()
							{
								// .get() didn't throw, so we succeeded, print out success message
								MessageDialog ^dialog = ref new MessageDialog(loader->GetString("FilesImportSuccessText"));
								dialog->ShowAsync();
							}));

						}
						catch (...)
						{
							// We'll handle the specific errors below.
						}


					});
					
				});

				//auto transform = ((UIElement^)sender)->TransformToVisual(nullptr); //nullptr to get position related to windows
				auto transform = ((UIElement^)titleBar)->TransformToVisual(nullptr);

				Windows::Foundation::Point point = transform->TransformPoint(Windows::Foundation::Point());
				statePopup->HorizontalOffset = point.X + 1; //+ selectStateBtn->ActualWidth / 2.0f - pane->Width / 2.0f;
				statePopup->VerticalOffset = point.Y + titleBar->ActualHeight;

				statePopup->IsOpen = true;
			});
		}
		else
			return create_task([] {});

	});

}







void ImportPage::importSavbtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FileOpenPicker ^picker = ref new FileOpenPicker();
	
	picker->FileTypeFilter->Append(".sav");
	picker->FileTypeFilter->Append(".sgm");
	

	picker->ViewMode = PickerViewMode::List;
	//picker->CommitButtonText = "Select";
	auto loader = Windows::ApplicationModel::Resources::ResourceLoader::GetForCurrentView();


	task<void> t = create_task(picker->PickSingleFileAsync()).then([this, loader](StorageFile ^file)
	{
		if (file)
		{
			//get the save name without extension
			Platform::String ^file_path = file->Path;
			wstring wfilepath(file_path->Begin(), file_path->End());

			wstring folderpath;
			wstring filename;
			wstring filenamenoext;
			wstring ext;
			splitFilePath(wfilepath, folderpath, filename, filenamenoext, ext);

			
			Platform::String^ pfilenamenoext = ref new Platform::String(filenamenoext.c_str());

			ROMDBEntry^ entry = App::ROMDB->GetEntryFromName(pfilenamenoext);

			if (entry == nullptr)
			{
				MessageDialog ^dialog = ref new MessageDialog(loader->GetString("NoMatchingROMError"), loader->GetString("ErrorText"));
				dialog->ShowAsync();
				return create_task([] {});
			}

			//if the file is ingame save then prevent autoloading
			if (ext == L"sav")
				entry->AutoLoadLastState = false;

			//copy the save file over
			Platform::String^ pfolderpath = ref new Platform::String(folderpath.c_str());
			if (pfolderpath == entry->Folder->Path)
			{
				MessageDialog ^dialog = ref new MessageDialog(loader->GetString("SaveFileImportSuccessText"));
				dialog->ShowAsync();
				return create_task([] {});
			}
			else
			{
				return create_task(file->CopyAsync(entry->Folder, file->Name, NameCollisionOption::ReplaceExisting))
					.then([loader](StorageFile^ file)
				{
					MessageDialog ^dialog = ref new MessageDialog(loader->GetString("SaveFileImportSuccessText"));
					dialog->ShowAsync();
					return create_task([] {});
				});
			}

		}
		else
			return create_task([] {});

	});
}




void ImportPage::Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//try re-sign in silently because access token expires every 1 hour
	if (EmulatorSettings::Current->SignedIn)
	{
		//live::live_client* LiveClient = new live::live_client();
		App::LiveClient->login(L"wl.skydrive_update wl.signin", true)
			.then([this](bool isLoggedIn)
		{
			signin_Completed(isLoggedIn);
		});
	}
}


void ImportPage::SignInbtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//live::live_client* LiveClient = new live::live_client();
	App::LiveClient->login(L"wl.skydrive_update wl.signin", false)
		.then([this](bool isLoggedIn)
	{
		signin_Completed(isLoggedIn);

	});
}

void ImportPage::signin_Completed(bool isLoggedIn)
{
	auto loader = Windows::ApplicationModel::Resources::ResourceLoader::GetForCurrentView();

	if (isLoggedIn)
	{
		this->SignInbtn->Content = loader->GetString("SignedInText");
		this->SignInbtn->IsEnabled = false;
		this->importOneDriveROMbtn->IsEnabled = true;
		EmulatorSettings::Current->SignedIn = true;
	}
	else
	{
		this->SignInbtn->Content = loader->GetString("SignInText");
		this->SignInbtn->IsEnabled = true;
		this->importOneDriveROMbtn->IsEnabled = false;
		EmulatorSettings::Current->SignedIn = false;
	}
}


void ImportPage::importOneDriveROMbtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Frame->Navigate(
		TypeName(FileBrowserPane::typeid),
		nullptr,
		ref new Windows::UI::Xaml::Media::Animation::DrillInNavigationTransitionInfo());

	////open panel to let user select file
	//Popup ^statePopup = ref new Popup();
	//statePopup->IsLightDismissEnabled = false;

	//FileBrowserPane ^pane = ref new FileBrowserPane();
	//statePopup->Child = pane;
	//pane->Width = titleBar->ActualWidth;//statePopup->Width;
	//pane->Height = Window::Current->Bounds.Height - 48; //statePopup->MaxHeight;

	////pane->FileSelectedCallback = ref new FileSelectedDelegate([=](StorageFile ^file)
	////{

	////});

	////auto transform = ((UIElement^)sender)->TransformToVisual(nullptr); //nullptr to get position related to windows
	//auto transform = ((UIElement^)titleBar)->TransformToVisual(nullptr);

	//Windows::Foundation::Point point = transform->TransformPoint(Windows::Foundation::Point());
	//statePopup->HorizontalOffset = point.X + 1; //+ selectStateBtn->ActualWidth / 2.0f - pane->Width / 2.0f;
	//statePopup->VerticalOffset = point.Y + titleBar->ActualHeight;

	//statePopup->IsOpen = true;
}
