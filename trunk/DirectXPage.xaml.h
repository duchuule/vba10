//
// DirectXPage.xaml.h
// Declaration of the DirectXPage class.
//

#pragma once

#include "DirectXPage.g.h"

#include "Common\DeviceResources.h"
#include "VBA10Main.h"

#include "NavMenuItem.h"
#include "NavMenuListView.h"


using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Media::Imaging;


namespace VBA10
{
	/// <summary>
	/// A page that hosts a DirectX SwapChainPanel.
	/// </summary>
	public ref class DirectXPage sealed
	{
	public:
		DirectXPage();
		virtual ~DirectXPage();

		
		void LoadInternalState(Windows::Foundation::Collections::IPropertySet^ state);

		//from AppShell
		property Windows::UI::Xaml::Controls::Frame^ AppFrame
		{
			Windows::UI::Xaml::Controls::Frame^ get();
		}
		void GoToPage(int pageindex);

		void LoadROM(StorageFile ^file, StorageFolder ^folder);
		void SaveState();
		void LoadState();
		void Reset();
		void SelectSaveState(int slot);
		

	//from AppShell
	internal:
		/// <summary>
		/// An event to notify listeners when the hamburger button may occlude other content in the app.
		/// The custom "PageHeader" user control is using this.
		/// </summary>
		event TypedEventHandler<DirectXPage^, Rect>^ TogglePaneButtonRectChanged;
		property Rect TogglePaneButtonRect
		{
			Rect get() { return _togglePaneButtonRect; }
		private:
			void set(Rect value) { _togglePaneButtonRect = value; }
		}

		static property DirectXPage^ Current
		{
			DirectXPage^ get()
			{
				return _current;
			}
		}

		task<void> SaveInternalState(Windows::Foundation::Collections::IPropertySet^ state);
		BitmapSource^ TakeSnapshot();
		task<void> SaveSnapshot();
	private:
		//variables
		bool loadingDialogOpen;
		StorageFile^ tmpfile;
		StorageFolder^ tmpfolder;

		//function
		task<void> CopyDemoROM(void);
		

		// XAML low-level rendering event handler.
		void OnRendering(Platform::Object^ sender, Platform::Object^ args);

		// Window event handlers.
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);

		// DisplayInformation event handlers.
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

		// Other event handlers.
		void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args);
		void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);

		//keyboard
		void AppShell_KeyDown(Object^ sender, KeyRoutedEventArgs^ e);

		// Track our independent input on a background worker thread.
		Windows::Foundation::IAsyncAction^ m_inputLoopWorker;
		Windows::UI::Core::CoreIndependentInputSource^ m_coreInput;

		//event for rendering
		Windows::Foundation::EventRegistrationToken m_eventToken;


		// Independent input handling functions.
		void OnPointerPressed(CoreWindow ^window, PointerEventArgs ^args);
		void OnPointerMoved(CoreWindow ^window, PointerEventArgs ^args);
		void OnPointerReleased(CoreWindow ^window, PointerEventArgs ^args);

		// Resources used to render the DirectX content in the XAML page background.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		std::unique_ptr<VBA10Main> m_main; 
		bool m_windowVisible;

		//from AppShell
		void NavMenuList_ItemInvoked(Object^ sender, ListViewItem^ e);
		void CheckTogglePaneButtonSizeChanged();
		void NavMenuItemContainerContentChanging(ListViewBase^ sender, ContainerContentChangingEventArgs^ args);

		Vector<NavMenuItem^>^ navlist;
		Rect _togglePaneButtonRect;

		static DirectXPage^ _current;
	
		void TogglePaneButton_UnChecked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void TogglePaneButton_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void CloseMenu();
	};
}

