//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"
#include <ppltasks.h>
#include "EmulatorFileHandler.h"
#include "SelectROMPane.xaml.h"
#include "Database\ROMDatabase.h"

#include "NavMenuItem.h"
#include "NavMenuListView.h"
#include "SelectROMPane.xaml.h"
#include "SettingsPage.xaml.h"
#include "HelpPage.xaml.h"
#include "CheatPane.xaml.h"
#include "ExportPage.xaml.h"
#include "ImportPage.xaml.h"



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

using namespace std;
using namespace VBA10;
using namespace VBA10::Controls;




DirectXPage^ DirectXPage::_current;

Frame^ DirectXPage::AppFrame::get()
{
	return frame;
}

DirectXPage::DirectXPage():
	m_windowVisible(true),
	m_coreInput(nullptr)
{
	InitializeComponent();

	DirectXPage::_current = this;

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



	//// Run task on a dedicated high priority background thread.
	//m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	// Declare the top level nav items
	navlist = ref new Vector<NavMenuItem^>();

	navlist->Append(
		ref new NavMenuItem(
			"Home",
			Symbol::Home,
			TypeName(SelectROMPane::typeid)));

	navlist->Append(
		ref new NavMenuItem(
			"Cheats",
			Symbol::Admin,
			TypeName(CheatPane::typeid)));

	navlist->Append(
		ref new NavMenuItem(
			"Import",
			Symbol::Download,
			TypeName(ImportPage::typeid)));

	navlist->Append(
		ref new NavMenuItem(
			"Export",
			Symbol::Upload,
			TypeName(ExportPage::typeid)));

	navlist->Append(
		ref new NavMenuItem(
			"Settings",
			Symbol::Setting,
			TypeName(SettingsPage::typeid)));

	navlist->Append(
		ref new NavMenuItem(
			"Help",
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

	//open the database
	App::ROMDB->Initialize().then([this, settings]
	{
		if (!settings->HasKey("FIRSTSTART"))
		{
			settings->Insert("FIRSTSTART", dynamic_cast<PropertyValue^>(PropertyValue::CreateBoolean(false)));

			//copy DEMO ROm then open menu
			return CopyDemoROM();
		}
		else
			return create_task([] {});
	//}).then([this]
	//{
		//test insert ROM
		//ROMDBEntry^ entry = ref new ROMDBEntry(0, "Test game", "testgame.gba", "D:\\ROM");
		//return App::ROMDB->Add(entry);

		//test obtain ROM
		//auto entry = App::ROMDB->AllROMDBEntries->GetAt(1);



	}).then([this]
	{
		//open menu
		RootSplitView->IsPaneOpen = true;
	});
	

}

task<void> DirectXPage::CopyDemoROM(void)
{
	StorageFolder ^installDir = Windows::ApplicationModel::Package::Current->InstalledLocation;
	return create_task(installDir->GetFolderAsync("Assets/")).then([](task<StorageFolder ^> t)
	{
		StorageFolder ^assetsFolder = t.get();
		return assetsFolder->GetFileAsync("Bunny Advance (Demo).gba");

	}).then([](StorageFile ^file)
	{
		//this file->DisplayName has extension

		//copy rom from installed dir to local folder
		return file->CopyAsync(ApplicationData::Current->LocalFolder);

	}).then([](StorageFile ^file)
	{
		//add entry to database and rom list
		ROMDBEntry^ entry = ref new ROMDBEntry(0, file->DisplayName, file->Name, file->Path); //this file->DisplayName has no extension

#if _DEBUG
		Platform::String ^message = file->DisplayName;
		wstring wstr(message->Begin(), message->End());
		OutputDebugStringW(wstr.c_str());
#endif


		App::ROMDB->AllROMDBEntries->Append(entry);
		return App::ROMDB->AddAsync(entry);

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
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->Trim();

	create_task(TakeSnapshot());

	// Stop rendering when the app is suspended.
	//m_main->StopRenderLoop();

	// Put code to save app state here.
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
	}
	else
	{
		//m_main->StopRenderLoop();
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
/// Check for the conditions where the navigation pane does not occupy the space under the floating
/// hamburger button and trigger the event.
/// Actually: this one is triggerd when the menu is closed and what it does is to feed information to Pageheader about the margin
/// </summary>
void DirectXPage::CheckTogglePaneButtonSizeChanged()
{
	if (RootSplitView->DisplayMode == SplitViewDisplayMode::Inline ||
		RootSplitView->DisplayMode == SplitViewDisplayMode::Overlay)
	{
		auto transform = TogglePaneButton->TransformToVisual(this);
		auto rect = transform->TransformBounds(Rect(0, 0, (float)TogglePaneButton->ActualWidth, (float)TogglePaneButton->ActualHeight));
		_togglePaneButtonRect = rect;
	}
	else
	{
		_togglePaneButtonRect = Rect();
	}

	TogglePaneButtonRectChanged(this, TogglePaneButtonRect);
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

	//change the size of app frame to zero to hide content
	AppFrame->Width = 0.0f;

	//unselect item
	NavMenuList->SetSelectedItem(nullptr);

	CheckTogglePaneButtonSizeChanged();

	//unpause emulator
	m_main->emulator->Unpause();
}

void DirectXPage::TogglePaneButton_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//pause emulator
	m_main->emulator->Pause();

	//change splitview to compact overlay and open pane
	//RootSplitView->DisplayMode = SplitViewDisplayMode::CompactOverlay;
	//RootSplitView->IsPaneOpen = true;

	//enable app frame
	AppFrame->IsEnabled = true;

	//change width to 100%, NAN means auto
	AppFrame->Width = NAN;

	//create screenshot
	TakeSnapshot();

	//navigate to the first item
	auto item = NavMenuList->ContainerFromItem(NavMenuList->Items->GetAt(0));
	NavMenuList->InvokeItem(item);

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

void DirectXPage::LoadROM(StorageFile ^file, StorageFolder ^folder)
{
	CloseMenu();
	LoadROMAsync(file, folder);

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

	LoadStateAsync().then([this]
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
	SelectSavestateSlot(slot);
}

task<void> DirectXPage::TakeSnapshot()
{
	if (IsROMLoaded())
	{
		//get the pixel information from buffer
		unsigned char *backbuffer;
		size_t pitch;
		int width, height;
		this->m_main->renderer->GetBackbufferData(&backbuffer, &pitch, &width, &height);
		Platform::Array<unsigned char> ^pixels = GetSnapshotBuffer(backbuffer, pitch, width, height);


		return create_task(ROMFolder->CreateFileAsync(ROMFile->DisplayName + ".jpg", CreationCollisionOption::OpenIfExists)
			).then([width, height, pixels](StorageFile ^file)
		{
			return file->OpenAsync(FileAccessMode::ReadWrite);
		}).then([width, height, pixels](IRandomAccessStream^ stream)
		{
			return BitmapEncoder::CreateAsync(BitmapEncoder::JpegEncoderId, stream);
		}).then([width, height, pixels](BitmapEncoder^ encoder)
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
	else
	{
		return create_task([] {});
	}

}