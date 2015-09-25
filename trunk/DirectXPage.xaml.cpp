//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage class.
//

#include "pch.h"
#include <robuffer.h>
#include "DirectXPage.xaml.h"
#include <ppltasks.h>
#include "EmulatorFileHandler.h"
#include "SelectROMPane.xaml.h"
#include "Database\ROMDatabase.h"
#include "Definitions.h"
#include "stringhelper.h"
#include "App.xaml.h"


#include "NavMenuItem.h"
#include "NavMenuListView.h"
#include "SelectROMPane.xaml.h"
#include "SettingsPage.xaml.h"
#include "HelpPage.xaml.h"
#include "CheatPane.xaml.h"
#include "ExportPage.xaml.h"
#include "ImportPage.xaml.h"
#include "PurchasePage.xaml.h"
#include "BlankPage.xaml.h"



using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace concurrency;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::FileProperties;
using namespace Windows::UI::ViewManagement;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Globalization; //to get date time
using namespace Windows::System::Display;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Storage::AccessCache;

using namespace std;
using namespace VBA10;
using namespace VBA10::Controls;




DirectXPage^ DirectXPage::_current;


DirectXPage::DirectXPage():
	m_windowVisible(true),
	m_coreInput(nullptr)
{
	InitializeComponent();

	DirectXPage::_current = this;

	//hide status bar on phone
	if (Windows::Foundation::Metadata::ApiInformation::IsTypePresent("Windows.UI.ViewManagement.StatusBar"))
	{
		Windows::UI::ViewManagement::StatusBar::GetForCurrentView()->HideAsync();
	}

	if (Windows::Foundation::Metadata::ApiInformation::IsTypePresent("Windows.Phone.UI.Input.HardwareButtons"))
	{
		//BackButton::Visibility = Visibility.Collapsed;
		Windows::Phone::UI::Input::HardwareButtons::BackPressed += ref new EventHandler<Windows::Phone::UI::Input::BackPressedEventArgs^>(this, &DirectXPage::OnHardwareBackButtonPressed);
	}


	// Register event handlers for page lifecycle.
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

	//we replace the pointer events by doing it on the UI thread
	Window::Current->CoreWindow->PointerPressed +=
		ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &DirectXPage::OnPointerPressed);

	Window::Current->CoreWindow->PointerMoved +=
		ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &DirectXPage::OnPointerMoved);

	Window::Current->CoreWindow->PointerReleased +=
		ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(this, &DirectXPage::OnPointerReleased);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged += 
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);

	m_eventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &DirectXPage::OnRendering));

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	m_deviceResources = std::make_shared<DX::DeviceResources>();
	m_deviceResources->SetSwapChainPanel(swapChainPanel);

	// Register our SwapChainPanel to get independent input pointer events
	//auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
	//{
	//	// The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
	//	m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
	//		Windows::UI::Core::CoreInputDeviceTypes::Mouse |
	//		Windows::UI::Core::CoreInputDeviceTypes::Touch |
	//		Windows::UI::Core::CoreInputDeviceTypes::Pen 
	//		);

	//	// Register for pointer events, which will be raised on the background thread.
	//	m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressed);
	//	m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
	//	m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);

	//	// Begin processing input messages as they're delivered.
	//	m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	//});

	//App::ESettings->LinearFilterEnabled = true;
	
	

	//// Run task on a dedicated high priority background thread.
	//m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	// Declare the top level nav items
	navlist = ref new Vector<NavMenuItem^>();

	auto loader = Windows::ApplicationModel::Resources::ResourceLoader::GetForCurrentView();
	navlist->Append(
		ref new NavMenuItem(
			loader->GetString("HomeText"),
			Symbol::Home,
			TypeName(SelectROMPane::typeid)));

	navlist->Append(
		ref new NavMenuItem(
			loader->GetString("CheatsText"),
			Symbol::ReportHacked,
			TypeName(CheatPane::typeid)));

	navlist->Append(
		ref new NavMenuItem(
			loader->GetString("ImportText"),
			Symbol::Download,
			TypeName(ImportPage::typeid)));

	navlist->Append(
		ref new NavMenuItem(
			loader->GetString("ExportText"),
			Symbol::Upload,
			TypeName(ExportPage::typeid)));

	navlist->Append(
		ref new NavMenuItem(
			loader->GetString("PurchaseText"),
			Symbol::Shop,
			TypeName(PurchasePage::typeid)));

	navlist->Append(
		ref new NavMenuItem(
			loader->GetString("SettingsText"),
			Symbol::Setting,
			TypeName(SettingsPage::typeid)));

	navlist->Append(
		ref new NavMenuItem(
			loader->GetString("HelpText"),
			Symbol::Help,
			TypeName(HelpPage::typeid)));

	NavMenuList->ItemsSource = navlist;
	//load settings
	auto settings = ApplicationData::Current->LocalSettings->Values;

	

	//initalize main object for rendering
	m_main = std::unique_ptr<VBA10Main>(new VBA10Main(m_deviceResources));

	//start rendering
	//DL: modified to not do it autmatically
	//m_main->StartRenderLoop();


	// create_task([this] {return CopyDemoROMAsync();}).then([this] //NOTE: this will make CopyDemoROM to run on background thread
	//and cause exception at entry->Snapshot->SetSourceAsync(stream);
	

	//For some reason, the first call to check xbox controller status always return false, so we call it first here,
	//then the next call in settings will return the true status.
	XINPUT_STATE state;
	ZeroMemory(&state,sizeof(XINPUT_STATE));

	for (int i = 0; i <= 3; i++)
		XInputGetState(i, &state);



	CopyDemoROMAsync()
		.then([this]   //NOTE: this let CopyDemonROM to run on UI thread
	{
		RootSplitView->IsPaneOpen = true;
		
	}, task_continuation_context::use_current());
	

}


void DirectXPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) 
{
}

void DirectXPage::OnHardwareBackButtonPressed(Platform::Object^ sender,
	Windows::Phone::UI::Input::BackPressedEventArgs ^args	)
{
	if (RootSplitView->IsPaneOpen)  
	{
		RootSplitView->IsPaneOpen = false;
		args->Handled = true;

		//args->Handled = false;  //default behavior, i.e. close the app

		
	}
	else
	{
		// The game is currently in active play mode, so hitting the hardware back button 
		// will cause the game to pause. 
		RootSplitView->IsPaneOpen = true;
		args->Handled = true;
	}
}

task<void> DirectXPage::CopyDemoROMAsync(void)
{
	auto settings = ApplicationData::Current->LocalSettings->Values;

	if (settings->HasKey("FIRSTSTART"))
		return concurrency::create_task([] {});
		


	settings->Insert("FIRSTSTART", dynamic_cast<PropertyValue^>(PropertyValue::CreateBoolean(false)));

	StorageFolder ^installDir = Windows::ApplicationModel::Package::Current->InstalledLocation;
	return concurrency::create_task(installDir->GetFolderAsync("Assets/"))
		.then([this](StorageFolder^ folder)
	{
		
		tmpfolder = folder;

		//get the rom file
		return tmpfolder->GetFileAsync("Bunny Advance (Demo).gba");

	}).then([this](StorageFile ^file)
	{
		//this file->DisplayName has extension

		//copy rom from installed dir to local folder
		return file->CopyAsync(ApplicationData::Current->LocalFolder);

	}).then([this](StorageFile ^file)
	{

#if _DEBUG
		Platform::String ^message = file->Path;
		wstring wstr(message->Begin(), message->End());
		OutputDebugStringW(wstr.c_str());
#endif
		//add entry to database and rom list
		ROMDBEntry^ entry = ref new ROMDBEntry(0, file->DisplayName, file->Name, ApplicationData::Current->LocalFolder->Path,
			"none","Bunny Advance (Demo).jpg"); //snapshot just need the file name

		entry->Folder = ApplicationData::Current->LocalFolder; //store pointer to folder

		


		App::ROMDB->AllROMDBEntries->Append(entry);
		return App::ROMDB->AddAsync(entry);
	}).then([this]
	{
		//get the snapshot file
		return tmpfolder->GetFileAsync("no_snapshot.png");

	}).then([this](StorageFile ^file)
	{
		//copy snapshot file to would be rom location
		return file->CopyAsync(ApplicationData::Current->LocalFolder, "Bunny Advance (Demo).jpg");

	}).then([this](StorageFile ^file)
	{
		//open file
		return file->OpenAsync(FileAccessMode::Read);
	}).then([this](IRandomAccessStream^ stream)
	{	
		//load bitmap image for snapshot
		auto entry = App::ROMDB->AllROMDBEntries->GetAt(0);
		entry->Snapshot = ref new BitmapImage();
		return entry->Snapshot->SetSourceAsync(stream);

	}).then([](task<void> t)
	{
		try
		{
			t.get();
			// .get() didn't throw, so we succeeded, 

		}
		catch (Platform::Exception ^ex)
		{
#if _DEBUG
			Platform::String ^message = ex->Message;
			wstring wstr(message->Begin(), message->End());
			OutputDebugStringW(wstr.c_str());
#endif
		}
	});
}

DirectXPage::~DirectXPage()
{
	// Stop rendering and processing events on destruction.
	//m_main->StopRenderLoop();
	m_coreInput->Dispatcher->StopProcessEvents();
}

// Saves the current state of the app for suspend and terminate events.
task<void> DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->Trim();

	
	// Stop rendering when the app is suspended.
	//m_main->StopRenderLoop();

	// Put code to save app state here.
	if (IsROMLoaded())
	{
		
		return SaveBeforeStop();
	}
	else
		return concurrency::create_task([] {});
}


task<void> DirectXPage::SaveBeforeStop()
{
	return concurrency::create_task([this] {
		//move saving stuff from StopROMAsync over here + add save snapshot
		if (IsROMLoaded())
		{
			
			m_main->emulator->Pause();

			//update last played time
			Calendar^ calendar = ref new Calendar();
			calendar->SetToNow();
			loadedEntry->LastPlayed = calendar->GetDateTime();

			//save stuff
			SaveSnapshot().wait();
			UpdateDBEntry().wait();
			SaveSRAMAsync().wait();

			int oldstate = SavestateSlot;
			SavestateSlot = AUTOSAVESTATE_SLOT;
			SaveStateAsync().wait();
			SavestateSlot = oldstate;

			



		}
	});
}

// Loads the current state of the app for resume events.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	// Put code to load app state here.

	// Start rendering when the app is resumed.
	//m_main->StartRenderLoop();
}

// Window event handlers.

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
		//need code to pause game here
		//m_main->StartRenderLoop();
		if (RootSplitView->IsPaneOpen) //pause if menu is open
			m_main->emulator->Pause();
		else
			m_main->emulator->Unpause();


		m_main->emulator->HidInput->StartListening();


	}
	else
	{
		//m_main->StopRenderLoop();
		m_main->emulator->Pause();

		m_main->emulator->HidInput->UnregisterFromInputReportEvent();

	}
}

// DisplayInformation event handlers.

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetDpi(sender->LogicalDpi);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->CreateWindowSizeDependentResources();
}


void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->ValidateDevice();
}


void DirectXPage::OnRendering(Object^ sender, Object^ args)
{
	//this->manager->SingleUpdate(this->emulator);
	m_main->Update();
	m_main->Render();
	m_main->Present();
	/*this->emulator->Update();
	this->emulator->Draw();*/
}

void DirectXPage::OnPointerPressed(CoreWindow ^window, PointerEventArgs ^args)
{
	// When the pointer is pressed begin tracking the pointer movement.
	m_main->emulator->GetVirtualController()->PointerPressed(args->CurrentPoint);

}

void DirectXPage::OnPointerMoved(CoreWindow ^window, PointerEventArgs ^args)
{
	// Update the pointer tracking code.
	m_main->emulator->GetVirtualController()->PointerMoved(args->CurrentPoint);

}

void DirectXPage::OnPointerReleased(CoreWindow ^window, PointerEventArgs ^args)
{
	// Stop tracking pointer movement when the pointer is released.
	m_main->emulator->GetVirtualController()->PointerReleased(args->CurrentPoint);
}

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetLogicalSize(e->NewSize);
	m_main->CreateWindowSizeDependentResources();
}






/// <summary>
/// Navigate to the Page for the selected <paramref name="listViewItem"/>.
/// </summary>
/// <param name="sender"></param>
/// <param name="listViewItem"></param>
void DirectXPage::NavMenuList_ItemInvoked(Object^ sender, ListViewItem^ listViewItem)
{
	auto item = (NavMenuItem^)((NavMenuListView^)(sender))->ItemFromContainer(listViewItem);

	if (item != nullptr)
	{
		//if (item->DestPage.Name != AppFrame->CurrentSourcePageType.Name)
		{
			AppFrame->Navigate(item->DestPage, item->Arguments);
		}
	}
}





/// <summary>
/// Enable accessibility on each nav menu item by setting the AutomationProperties.Name on each container
/// using the associated Label of each item.
/// </summary>
/// <param name="sender"></param>
/// <param name="args"></param>
void DirectXPage::NavMenuItemContainerContentChanging(ListViewBase^ sender, ContainerContentChangingEventArgs^ args)
{
	if (!args->InRecycleQueue && args->Item != nullptr && dynamic_cast<NavMenuItem^>(args->Item) != nullptr)
	{
		args->ItemContainer->SetValue(Windows::UI::Xaml::Automation::AutomationProperties::NameProperty, ((NavMenuItem^)args->Item)->Label);
	}
	else
	{
		args->ItemContainer->ClearValue(Windows::UI::Xaml::Automation::AutomationProperties::NameProperty);
	}
}


/// <summary>
/// Callback when the SplitView's Pane is toggled close.  When the Pane is not visible
/// then the floating hamburger may be occluding other content in the app unless it is aware.
/// </summary>
/// <param name="sender"></param>
/// <param name="e"></param>
void DirectXPage::TogglePaneButton_UnChecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//change splitview to overlay, so that it disappear
	//RootSplitView->DisplayMode = SplitViewDisplayMode::Overlay;

	//disable AppFrame so that it does not receive input accidentally
	AppFrame->IsEnabled = false;

	//navigate to a blank page to avoid ad control reload
	AppFrame->Navigate(TypeName(BlankPage::typeid));

	//change the size of app frame to zero to hide content
	AppFrame->Width = 0.0f;

	//hide the hamburger button
	if (EmulatorSettings::Current->HideHamburger)
		this->TogglePaneButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	

	//unselect item
	//NavMenuList->SetSelectedItem(nullptr);

	//save and apply cheats if needed
	if (ShouldApplyNewCheats)
	{
		ApplyCheats(ROMCheats);
		SaveCheats();
	}

	//CheckTogglePaneButtonSizeChanged();

	//unpause emulator
	m_main->emulator->Unpause();

	m_main->emulator->HidInput->StartListening();


	//prevent screen lock when game is playing
	try
	{
		if (g_DisplayRequest == nullptr) {
			// This call creates an instance of the displayRequest object 
			g_DisplayRequest = ref new DisplayRequest();
		}

		if (g_DisplayRequest != nullptr) 
		{
				// This call activates a display-required request. If successful,  
				// the screen is guaranteed not to turn off automatically due to user inactivity. 
				g_DisplayRequest->RequestActive();

		}
	}
	catch (...) {}
}

void DirectXPage::TogglePaneButton_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

	//enable screen lock again
	try
	{
		if (g_DisplayRequest != nullptr)
		{
			// This call activates a display-required request. If successful,  
			// the screen is guaranteed not to turn off automatically due to user inactivity. 
			g_DisplayRequest->RequestRelease();

		}
	}
	catch (...) {}

	m_main->emulator->HidInput->StopListening();

	//pause emulator
	m_main->emulator->Pause();

	//change splitview to compact overlay and open pane
	//RootSplitView->DisplayMode = SplitViewDisplayMode::CompactOverlay;
	//RootSplitView->IsPaneOpen = true;


	

	if (IsROMLoaded())
	{

		//calculate snapshot name
		Platform::String ^file_path = ROMFile->Path;
		wstring wfilepath(file_path->Begin(), file_path->End());

		wstring folderpath;
		wstring filename;
		wstring filenamenoext;
		wstring ext;
		splitFilePath(wfilepath, folderpath, filename, filenamenoext, ext);

		wstring snapshotpath = folderpath + L"\\" + filenamenoext + L".jpg";
		Platform::String^ psnapshotname = ref new Platform::String(snapshotpath.c_str());


		// new snapshot
		loadedEntry->Snapshot = TakeSnapshot();

		//update last played time
		Calendar^ calendar = ref new Calendar();
		calendar->SetToNow();
		loadedEntry->LastPlayed = calendar->GetDateTime();

	}
	//enable app frame
	AppFrame->IsEnabled = true;

	//change width to 100%, NAN means auto
	AppFrame->Width = NAN;


	//show hamburber button
	this->TogglePaneButton->Visibility = Windows::UI::Xaml::Visibility::Visible;



	//navigate to the first item

	//GoToPage(0);
	int test = NavMenuList->SelectedIndex;
	if (!IsROMLoaded() || NavMenuList->SelectedIndex < 0)  //always go to page 0 if no rom has been started
		GoToPage(0);
	else //reload current page
		GoToPage(NavMenuList->SelectedIndex); //even when ROM is loaded, we force the Pane to reload


	

}

void DirectXPage::GoToPage(int pageindex)
{
	auto item = NavMenuList->ContainerFromItem(NavMenuList->Items->GetAt(pageindex));
	NavMenuList->InvokeItem(item);
}


void DirectXPage::AppShell_KeyDown(Object^ sender, KeyRoutedEventArgs^ e)
{
	if (e->Key == VirtualKey::Escape )
	{
		RootSplitView->IsPaneOpen = !RootSplitView->IsPaneOpen;

	}
	m_main->emulator->GetVirtualController()->Reset();

}

void DirectXPage::CloseMenu()
{
	

	RootSplitView->IsPaneOpen = false; //this will toggle the hamburger menu because of the 2-way binding
	m_main->emulator->GetVirtualController()->Reset();
}

void DirectXPage::LoadROM(ROMDBEntry^ entry)
{
	CloseMenu();
	loadedEntry = entry; //store loaded entry for later use
	SavestateSlot = entry->LastSaveIndex;



	if (IsROMLoaded() && entry->FolderPath + L"\\" + entry->FileName == ROMFile->Path) //don't have to do anything
		return;

	//NOTE: create task like this will put the task on the same thread (UI thread)
	//create_task(SaveBeforeStop())

	//create task like this will put the task on background thread
	concurrency::create_task([this, entry] {
		if (IsROMLoaded() && entry->FolderPath + L"\\" + entry->FileName != ROMFile->Path)  //different rom, save old rom state
		{
			return SaveBeforeStop();

		}
		else
			return concurrency::create_task([] {});
	}).then([entry] {
		return entry->Folder->GetFileAsync(entry->FileName);

	}).then([entry] (StorageFile^ file){ 
		return LoadROMAsync(file, entry->Folder);

	}).then([entry] {
		if (entry->AutoLoadLastState)
			return LoadStateAsync(AUTOSAVESTATE_SLOT);
		else
			return concurrency::create_task([] {});
	});
	//this is OK after we fixed the ParseVBAiniAsync so that it does not branch to another thread but it makes the UI unreponsive
	//LoadROMAsync(file, folder).then([this]
	//{
	//	CloseMenu();
	//});


}



void DirectXPage::SaveState()
{
	SaveStateAsync().then([this]
	{
		CloseMenu();
	});
}

void DirectXPage::LoadState()
{

	LoadStateAsync(SavestateSlot).then([this]
	{
		CloseMenu();
	});
}

void DirectXPage::Reset()
{
	CloseMenu();
	ResetSync();
}

void DirectXPage::SelectSaveState(int slot)
{
	loadedEntry->LastSaveIndex = slot;
	SelectSavestateSlot(slot);
}


BitmapSource^ DirectXPage::TakeSnapshot()
{
	//get the pixel information from buffer
	unsigned char *backbuffer;
	size_t pitch;
	int width, height;
	this->m_main->renderer->GetBackbufferData(&backbuffer, &pitch, &width, &height);
	Platform::Array<unsigned char> ^pixels = GetSnapshotBuffer(backbuffer, pitch, width, height);

	WriteableBitmap^ bitmap = ref new WriteableBitmap(width, height);

	// Cast to Object^, then to its underlying IInspectable interface.
	Object^ obj = bitmap->PixelBuffer;
	ComPtr<IInspectable> insp(reinterpret_cast<IInspectable*>(obj));

	// Query the IBufferByteAccess interface.
	ComPtr<IBufferByteAccess> bufferByteAccess;
	insp.As(&bufferByteAccess);

	// Retrieve the buffer data.
	byte* pPixels = nullptr;
	bufferByteAccess->Buffer(&pPixels);

	//not pixels store image in RGBA format while pPixels store image in BGRA format
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			pPixels[(x + y * width) * 4] = pixels[(x + y * width) * 4 + 2]; // B
			pPixels[(x + y * width) * 4 + 1] = pixels[(x + y * width) * 4 + 1]; // G
			pPixels[(x + y * width) * 4 + 2] = pixels[(x + y * width) * 4 ]; // R
			pPixels[(x + y * width) * 4 + 3] = pixels[(x + y * width) * 4 + 3]; // A
		}
	}

	return bitmap;

}

task<void> DirectXPage::SaveSnapshot()
{

	//get the pixel information from buffer
	unsigned char *backbuffer;
	size_t pitch;
	int width, height;
	this->m_main->renderer->GetBackbufferData(&backbuffer, &pitch, &width, &height);
	Platform::Array<unsigned char> ^pixels = GetSnapshotBuffer(backbuffer, pitch, width, height);

	//calculate snapshot name
	Platform::String ^file_path = ROMFile->Path;
	wstring wfilepath(file_path->Begin(), file_path->End());

	wstring folderpath;
	wstring filename;
	wstring filenamenoext;
	wstring ext;
	splitFilePath(wfilepath, folderpath, filename, filenamenoext, ext);
	
	wstring snapshotname = filenamenoext + L".jpg";
	Platform::String^ psnapshotname = ref new Platform::String(snapshotname.c_str());

	

	return concurrency::create_task(ROMFolder->CreateFileAsync(psnapshotname, CreationCollisionOption::OpenIfExists)
		).then([this, width, height, pixels](StorageFile ^file)
	{
		tmpfile = file;
		return file->OpenAsync(FileAccessMode::ReadWrite);
	}).then([this, width, height, pixels](IRandomAccessStream^ stream)
	{
		return BitmapEncoder::CreateAsync(BitmapEncoder::JpegEncoderId, stream);
	}).then([this, width, height, pixels](BitmapEncoder^ encoder)
	{
		encoder->SetPixelData(BitmapPixelFormat::Rgba8, BitmapAlphaMode::Ignore, width, height, 72.0f, 72.0f, pixels);
		return encoder->FlushAsync();

	}).then([](task<void> t)
	{
		try
		{
			t.get();
		}
		catch (COMException ^ex)
		{
		}
	});
}


task<void> DirectXPage::UpdateDBEntry()
{
	return 	App::ROMDB->UpdateAsync(loadedEntry);

}

void DirectXPage::ImportRomFromFile(FileActivatedEventArgs^ args)
{
	StorageFile^ file = (StorageFile^)(args->Files->GetAt(0));

	//calculate folder path and  snapshot name
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
	Platform::String^ pfolderpath = ref new Platform::String(folderpath.c_str());

	//calculate token
	replace(folderpath.begin(), folderpath.end(), ':', '_');
	replace(folderpath.begin(), folderpath.end(), '/', '_');
	replace(folderpath.begin(), folderpath.end(), '\\', '_');
	Platform::String^ ptoken = ref new Platform::String(folderpath.c_str());

	//add folder to future accesslist
	int locationType = 0; //default to private storage
	if (StorageApplicationPermissions::FutureAccessList->ContainsItem(ptoken))
		locationType = 1; //set to user accessible storage

	//check to see if this file is already in the database
	bool exist = false;
	ROMDBEntry^ entry = nullptr;
	for (int j = 0; j < App::ROMDB->AllROMDBEntries->Size; j++)
	{
		entry = App::ROMDB->AllROMDBEntries->GetAt(j);
		if ( (locationType == 0 && entry->FileName == file->Name)
			|| (locationType == 1 && entry->FileName == file->Name && entry->Token == ptoken))
		{
			exist = true;
			break;
		}
	}
	//=====create rom entry
	if (!exist)
	{
		if (locationType == 0)
			entry = ref new ROMDBEntry(locationType, pfilenamenoext, file->Name, ApplicationData::Current->LocalFolder->Path, "none", psnapshotname);
		else
			entry = ref new ROMDBEntry(locationType, pfilenamenoext, file->Name, pfolderpath, ptoken, psnapshotname);
		App::ROMDB->AllROMDBEntries->Append(entry);
	}

	vector<task<void>> tasks;
	

	//copy rom file over if we store in private storage (no matter if rom already exists)
	if (locationType == 0)
		tasks.emplace_back(
			concurrency::create_task(file->CopyAsync(ApplicationData::Current->LocalFolder, file->Name, NameCollisionOption::ReplaceExisting))
			.then([](task<StorageFile^> t)
		{
			try
			{
				t.get();

			}
			catch (...)
			{
			}
		}));

	if (!exist)
	{
		//=====copy snapshot over
		
		tasks.emplace_back(
			concurrency::create_task([locationType, ptoken, entry]	
			{
				if (locationType == 0)
				{
					entry->Folder = ApplicationData::Current->LocalFolder;
					return create_task([] {});
				}
				else
				{
					return concurrency::create_task([ptoken]() {
						return StorageApplicationPermissions::FutureAccessList->GetFolderAsync(ptoken);
					}).then([entry](StorageFolder^ folder) {
						entry->Folder = folder;
					});
				}

			}).then([entry] ()
			{
				StorageFolder ^installDir = Windows::ApplicationModel::Package::Current->InstalledLocation;
				return installDir->GetFolderAsync("Assets/");
			}, task_continuation_context::use_current()).then([entry](StorageFolder^ assetFolder)
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

		);//end of tasks.emplace_back

		

		//auto tmpentry = make_shared<ROMDBEntry^>(entry);


		//====write to database file
		tasks.emplace_back(	concurrency::create_task(App::ROMDB->AddAsync(entry))
		.then([](task<void> t)
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

	//when all finish
	when_all(begin(tasks), end(tasks)).then([this, entry](task<void> t)
	{
		try
		{
			t.get();
			LoadROM(entry);

		}
		catch (...)
		{
			// We'll handle the specific errors below.
		}


	}, task_continuation_context::use_current());
}


void DirectXPage::EditButtonLayout()
{
	this->m_main->emulator->EnterButtonEditMode();
	CloseMenu();
	this->panelEditButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
	
}

void DirectXPage::CancelPositionBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->m_main->emulator->LeaveButtonEditMode(false);
	this->panelEditButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}


void DirectXPage::AcceptPositionBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->m_main->emulator->LeaveButtonEditMode(true);
	this->panelEditButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}
