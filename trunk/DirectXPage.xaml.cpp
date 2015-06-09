//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"
#include <ppltasks.h>
#include "EmulatorFileHandler.h"
#include "SelectROMPane.xaml.h"

using namespace std;
using namespace VBA10;

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
using namespace concurrency;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::FileProperties;
using namespace Windows::UI::ViewManagement;

#define SETTINGS_WIDTH			346

extern bool enableTurboMode;

DirectXPage::DirectXPage():
	m_windowVisible(true),
	m_coreInput(nullptr)
{
	InitializeComponent();

	// Register event handlers for page lifecycle.
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

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

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	m_deviceResources = std::make_shared<DX::DeviceResources>();
	m_deviceResources->SetSwapChainPanel(swapChainPanel);

	// Register our SwapChainPanel to get independent input pointer events
	auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
	{
		// The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
		m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
			Windows::UI::Core::CoreInputDeviceTypes::Mouse |
			Windows::UI::Core::CoreInputDeviceTypes::Touch |
			Windows::UI::Core::CoreInputDeviceTypes::Pen
			);

		// Register for pointer events, which will be raised on the background thread.
		m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressed);
		m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
		m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);

		// Begin processing input messages as they're delivered.
		m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	});

	// Run task on a dedicated high priority background thread.
	m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	//load settings
	auto settings = ApplicationData::Current->LocalSettings->Values;

	//copy DEMO ROm
	if (!settings->HasKey("FIRSTSTART"))
	{
		settings->Insert("FIRSTSTART", dynamic_cast<PropertyValue^>(PropertyValue::CreateBoolean(false)));
		this->CopyDemoROM();
	}

	//initalize main object for rendering
	m_main = std::unique_ptr<VBA10Main>(new VBA10Main(m_deviceResources));

	//start rendering
	//DL: modified to not do it autmatically
	//m_main->StartRenderLoop();
}

void DirectXPage::CopyDemoROM(void)
{
	StorageFolder ^installDir = Windows::ApplicationModel::Package::Current->InstalledLocation;
	create_task(installDir->GetFolderAsync("Assets/")).then([](task<StorageFolder ^> t)
	{
		StorageFolder ^assetsFolder = t.get();
		return assetsFolder->GetFileAsync("Bunny Advance (Demo).gba");

	}).then([](StorageFile ^file)
	{

		file->CopyAsync(ApplicationData::Current->LocalFolder);

#if _DEBUG
		wstring wstr((ApplicationData::Current->LocalFolder->Path->Data()));
		OutputDebugStringW((wstr + L"\n").c_str());
#endif
	}).then([](task<void> t)
	{
		try
		{
			t.get();
			// .get() didn't throw, so we succeeded.
			OutputDebugStringW(L"File copied\n");
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
	m_main->StopRenderLoop();
	m_coreInput->Dispatcher->StopProcessEvents();
}

// Saves the current state of the app for suspend and terminate events.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->Trim();

	// Stop rendering when the app is suspended.
	m_main->StopRenderLoop();

	// Put code to save app state here.
}

// Loads the current state of the app for resume events.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	// Put code to load app state here.

	// Start rendering when the app is resumed.
	m_main->StartRenderLoop();
}

// Window event handlers.

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
		//need code to pause game here
		m_main->StartRenderLoop();
	}
	else
	{
		m_main->StopRenderLoop();
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



void DirectXPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
	// When the pointer is pressed begin tracking the pointer movement.

}

void DirectXPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
	// Update the pointer tracking code.

}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
	// Stop tracking pointer movement when the pointer is released.

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




void DirectXPage::StartROM_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	////pause emulator 
	//m_main->emulator->Pause();

	//StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;
	//String^ name = "Bunny Advance (Demo).gba";
	//create_task(localFolder->GetFileAsync(name)).then([=](StorageFile^ romFile) {
	//	//Do something with the rom file 
	//	LoadROMAsync(romFile, localFolder);
	//});

	if (this->loadingDialogOpen)
		return;

	Popup ^selectRomPopup = ref new Popup();
	selectRomPopup->Width = Window::Current->Bounds.Width - 200;
	selectRomPopup->Height = Window::Current->Bounds.Height - 200;
	selectRomPopup->IsLightDismissEnabled = false;

	SelectROMPane ^pane = ref new SelectROMPane();
	pane->Width = Window::Current->Bounds.Width * 0.95;
	pane->Height = Window::Current->Bounds.Height * 0.95;
	selectRomPopup->Child = pane;

	selectRomPopup->Opened += ref new EventHandler<Object ^>([this](Object ^sender, Object ^args)
	{
		m_main->emulator->Pause();
	});
	selectRomPopup->Closed += ref new EventHandler<Object ^>([this, pane](Object ^sender, Object ^args)
	{
		if (pane->Cancelled)
		{
			m_main->emulator->Unpause();
		}
		this->loadingDialogOpen = false;
		m_main->emulator->GetVirtualController()->Reset();
	});

	pane->ROMSelected = ref new ROMSelectedDelegate([=](StorageFile ^file, StorageFolder ^folder)
	{
		LoadROMAsync(file, folder);
		this->BottomAppBar->IsOpen = false;
		this->loadingDialogOpen = false;
	});

	selectRomPopup->SetValue(Canvas::LeftProperty, 100);
	selectRomPopup->SetValue(Canvas::TopProperty, 100);
	selectRomPopup->IsOpen = true;
	this->loadingDialogOpen = true;

	
}
