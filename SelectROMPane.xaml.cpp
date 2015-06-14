//
// SelectROMPane.xaml.cpp
// Implementation of the SelectROMPane class
//

#include "pch.h"
#include "SelectROMPane.xaml.h"
#include <Windows.h>
#include "DirectXPage.xaml.h";
#include "EmulatorSettings.h";
#include "EmulatorFileHandler.h"
#include "SelectStatePane.xaml.h"

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

StorageFolderModel::StorageFolderModel(StorageFolder ^folder)
{ 
	this->Folder = folder;
}

StorageFileModel::StorageFileModel(StorageFile ^file, StorageFolder ^folder)
{
	this->File = file;
	this->Folder = folder;
}

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236
SelectROMPane::SelectROMPane()
{

	this->InitializeComponent();
	this->InitializeStorageLists();

	//disable the command bar if no rom is loaded
	topbar->IsEnabled = IsROMLoaded();
}

void SelectROMPane::InitializeStorageLists(void)
{
	this->storageFileVector = ref new Platform::Collections::Vector<StorageFileModel ^>();
	this->storageFolderVector = ref new Platform::Collections::Vector<StorageFolderModel ^>();
	AccessListEntryView ^folderList = StorageApplicationPermissions::FutureAccessList->Entries;

	if(folderList->Size == 0)
	{
		this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
		{
			this->romDirList->ItemsSource = this->storageFolderVector;
			this->RefreshROMList();
		}));
		return;
	}
	
	task<void> prevTask = create_task([folderList]()
	{
		return StorageApplicationPermissions::FutureAccessList->GetFolderAsync(folderList->GetAt(0).Token);
	}).then([this](StorageFolder ^folder)
	{
		this->storageFolderVector->Append(ref new StorageFolderModel(folder));
	}).then([this](task<void> t)
	{
		try
		{
			t.get();
		}catch(Platform::COMException ^ex)
		{
#if _DEBUG
			OutputDebugStringW(L"Folder in FutureAccessList does not exist anymore.");
#endif
		}
		catch(...)
		{
#if _DEBUG
			OutputDebugStringW(L"Folder in FutureAccessList does not exist anymore.");
#endif
		}
	});

	for (int i = 1; i < folderList->Size; i++)
	{
		prevTask = prevTask.then([this, folderList, i]()
		{
			return StorageApplicationPermissions::FutureAccessList->GetFolderAsync(folderList->GetAt(i).Token);
		}).then([this](StorageFolder ^folder)
		{
			this->storageFolderVector->Append(ref new StorageFolderModel(folder));
		}).then([this](task<void> t)
		{
			try
			{
				t.get();
			}catch(Platform::COMException ^ex)
			{
#if _DEBUG
				OutputDebugStringW(L"Folder in FutureAccessList does not exist anymore.");
#endif
			}
			catch(...)
			{
#if _DEBUG
				OutputDebugStringW(L"Folder in FutureAccessList does not exist anymore.");
#endif
			}
		});
	}

	prevTask.then([this]()
	{
		this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
		{
			this->romDirList->ItemsSource = this->storageFolderVector;
			this->RefreshROMList();
		}));
	});	
}


void SelectROMPane::Close()
{
	try
	{
		(safe_cast<Popup ^>(this->Parent))->IsOpen = false;
	}catch(InvalidCastException ^ex)
	{
#if _DEBUG
		Platform::String ^message = ex->Message;
		wstring wstr(message->Begin(), message->End());
		OutputDebugStringW(L"InvalidCastException");
#endif
	}
}

void SelectROMPane::AddRomDirectoryClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FolderPicker ^picker = ref new FolderPicker();
	
#ifndef GBC
	picker->FileTypeFilter->Append(".gba");
#else
	picker->FileTypeFilter->Append(".gbc");
	picker->FileTypeFilter->Append(".gb");
#endif
	picker->SuggestedStartLocation = PickerLocationId::ComputerFolder;
	picker->ViewMode = PickerViewMode::List;
	picker->CommitButtonText = "Select ROM Directory";
	
	task<void> t = create_task(picker->PickSingleFolderAsync()).then([this](StorageFolder ^folder)
	{
		if(folder)
		{
			bool contained = StorageApplicationPermissions::FutureAccessList->ContainsItem(folder->DisplayName);
			StorageApplicationPermissions::FutureAccessList->AddOrReplace(folder->DisplayName, folder); 
			if(contained)
			{
				this->RemoveFolderByToken(folder->DisplayName);
			}
			this->storageFolderVector->Append(ref new StorageFolderModel(folder));
		}
	}).then([this]()
	{
		this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
		{
			this->romDirList->ItemsSource = this->storageFolderVector;
			this->RefreshROMList();
		}));
	});
}

void SelectROMPane::DeleteFolderClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Button ^button = safe_cast<Button ^>(sender);
	StorageFolderModel ^model = safe_cast<StorageFolderModel ^>(button->DataContext);
	StorageApplicationPermissions::FutureAccessList->Remove(model->Name);
	this->RemoveFolderByToken(model->Name);
	this->romDirList->ItemsSource = this->storageFolderVector;
	this->RefreshROMList();
}

void SelectROMPane::RemoveFolderByToken(Platform::String ^token)
{
	for (int i = 0; i < this->storageFolderVector->Size; i++)
	{
		if(this->storageFolderVector->GetAt(i)->Name == token)
		{
			this->storageFolderVector->RemoveAt(i);
			break;
		}
	}
}

void SelectROMPane::RefreshROMList(void)
{
	this->storageFileVector->Clear();

	if(this->storageFolderVector->Size == 0)
	{
		StorageFolder ^localfolder = ApplicationData::Current->LocalFolder;
		create_task([localfolder]()
		{
#ifndef GBC
			return localfolder->GetFileAsync("Bunny Advance (Demo).gba");
#else
			return localfolder->GetFileAsync("Pong.gb");
#endif
		}).then([this, localfolder](StorageFile ^file)
		{
			StorageFileModel ^model = ref new StorageFileModel(file, localfolder);
			this->storageFileVector->Append(model);
		}).then([this]()
		{
			this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
			{
				this->romList->ItemsSource = this->storageFileVector;
			}));
		}).then([](task<void> t)
		{
			try
			{
				t.get();
			}catch(...)
			{
	#if _DEBUG
				OutputDebugStringW(L"Cannot find demo ROM.");
	#endif
			}
		});
		return;
	}

	Search::QueryOptions ^options = ref new Search::QueryOptions();
#ifndef GBC
	options->FileTypeFilter->Append(".gba");
#else
	options->FileTypeFilter->Append(".gbc");
	options->FileTypeFilter->Append(".gb");
#endif
	
	
	StorageFolder ^folder = this->storageFolderVector->GetAt(0)->Folder;
	task<void> t = create_task([this, folder, options]()
	{
		return folder->CreateFileQueryWithOptions(options)->GetFilesAsync();		
	}).then([this, folder](IVectorView<StorageFile ^> ^files)
	{
		for (int i = 0; i < files->Size; i++)
		{
			StorageFileModel ^model = ref new StorageFileModel(files->GetAt(i), folder);
			this->storageFileVector->Append(model);
		}
	});

	for (int i = 1; i < this->storageFolderVector->Size; i++)
	{
		StorageFolder ^folder = this->storageFolderVector->GetAt(i)->Folder;
		t = t.then([this, folder, options]()
		{
			return folder->CreateFileQueryWithOptions(options)->GetFilesAsync();		
		}).then([this, folder](IVectorView<StorageFile ^> ^files)
		{
			for (int i = 0; i < files->Size; i++)
			{
				StorageFileModel ^model = ref new StorageFileModel(files->GetAt(i), folder);
				this->storageFileVector->Append(model);
			}
		});
	}
	
	StorageFolder ^localfolder = ApplicationData::Current->LocalFolder;
	t = t.then([localfolder]()
	{
#ifndef GBC
		return localfolder->GetFileAsync("Bunny Advance (Demo).gba");
#else
		return localfolder->GetFileAsync("Pong.gb");
#endif
	}).then([this, localfolder](StorageFile ^file)
	{
		StorageFileModel ^model = ref new StorageFileModel(file, localfolder);
		this->storageFileVector->Append(model);
	}).then([](task<void> t)
	{
		try
		{
			t.get();
		}catch(...)
		{
#if _DEBUG
			OutputDebugStringW(L"Cannot find demo ROM.");
#endif
		}
	});

	t.then([this]()
	{
		this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
		{
			this->romList->ItemsSource = this->storageFileVector;
		}));
	});
}


void SelectROMPane::romList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	StorageFileModel ^model =	safe_cast<StorageFileModel ^>(this->romList->SelectedValue);


	DirectXPage::Current->LoadROM(model->File, model->Folder);
}


void SelectROMPane::saveBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (IsSaveConfirmationDisabled())
	{
		DirectXPage::Current->SaveState();
	}
	else
	{
		MessageDialog ^dialog = ref new MessageDialog("Are you sure you want to save? This will overwrite the selected save state.", "Overwrite?");
		UICommand ^confirm = ref new UICommand("Yes",
			ref new UICommandInvokedHandler([this](IUICommand ^cmd)
		{
			DirectXPage::Current->SaveState();
		}));

		UICommand ^confirmRemember = ref new UICommand("Yes, don't ask again",
			ref new UICommandInvokedHandler([this](IUICommand ^cmd)
		{
			DisableSaveConfirmation(true);
			DirectXPage::Current->SaveState();
		}));

		UICommand ^no = ref new UICommand("No",
			ref new UICommandInvokedHandler([this](IUICommand ^cmd)
		{
			//do nothing
		}));

		dialog->Commands->Append(confirm);
		//dialog->Commands->Append(confirmRemember); //windows phone crashes when there are 3 options
		dialog->Commands->Append(no);

		dialog->DefaultCommandIndex = 0;
		dialog->CancelCommandIndex = 1; //would be two if confirm remember is used

		dialog->ShowAsync();
	}
}


void SelectROMPane::loadBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (IsLoadConfirmationDisabled())
	{
		DirectXPage::Current->LoadState();
	}
	else
	{
		MessageDialog ^dialog = ref new MessageDialog("Are you sure you want to load? All unsaved progress will be lost.", "Warning");
		UICommand ^confirm = ref new UICommand("Yes",
			ref new UICommandInvokedHandler([this](IUICommand ^cmd)
		{
			DirectXPage::Current->LoadState();
		}));

		UICommand ^confirmRemember = ref new UICommand("Yes, don't ask again",
			ref new UICommandInvokedHandler([this](IUICommand ^cmd)
		{
			DisableLoadConfirmation(true);
			DirectXPage::Current->LoadState();
		}));

		UICommand ^no = ref new UICommand("No",
			ref new UICommandInvokedHandler([this](IUICommand ^cmd)
		{
			//do nothing
		}));

		dialog->Commands->Append(confirm);
		//dialog->Commands->Append(confirmRemember);
		dialog->Commands->Append(no);

		dialog->DefaultCommandIndex = 0;
		dialog->CancelCommandIndex = 1;

		dialog->ShowAsync();
	}
}


void SelectROMPane::resetBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	MessageDialog ^dialog = ref new MessageDialog("Are you sure you want to reset the console?", "Warning");
	UICommand ^confirm = ref new UICommand("Yes",
		ref new UICommandInvokedHandler([this](IUICommand ^cmd)
	{
		DirectXPage::Current->Reset();
	}));


	UICommand ^no = ref new UICommand("No",
		ref new UICommandInvokedHandler([this](IUICommand ^cmd)
	{
		//do nothing
	}));

	dialog->Commands->Append(confirm);
	dialog->Commands->Append(no);

	dialog->DefaultCommandIndex = 0;
	dialog->CancelCommandIndex = 1;

	dialog->ShowAsync();
}


void SelectROMPane::selectStateBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Popup ^statePopup = ref new Popup();
	statePopup->IsLightDismissEnabled = true;

	SelectStatePane ^pane = ref new SelectStatePane(GetSavestateSlot());
	statePopup->Child = pane;
	//pane->Width = 200;//statePopup->Width;
	pane->MaxHeight = Window::Current->Bounds.Height - 48; //statePopup->MaxHeight;

	
	//auto transform = ((UIElement^)sender)->TransformToVisual(nullptr); //nullptr to get position related to windows
	auto transform = ((UIElement^)topbar)->TransformToVisual(nullptr);

	Windows::Foundation::Point point = transform->TransformPoint(Windows::Foundation::Point());
	statePopup->HorizontalOffset = point.X; //+ selectStateBtn->ActualWidth / 2.0f - pane->Width / 2.0f;
	statePopup->VerticalOffset = point.Y + selectStateBtn->ActualHeight; 

	//statePopup->Measure(Windows::Foundation::Size(Window::Current->Bounds.Width, Window::Current->Bounds.Height));
	//statePopup->SetValue(Canvas::LeftProperty, Window::Current->Bounds.Left);
	//statePopup->SetValue(Canvas::TopProperty, this->windowBounds.Height - (88 + pane->DesiredSize.Height));


	statePopup->IsOpen = true;
}
