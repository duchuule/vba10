//
// AdControl.xaml.cpp
// Implementation of the AdControl class
//

#include "pch.h"
#include "AdControl.xaml.h"

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
using namespace Windows::UI::Core;


// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

AdControl::AdControl()
{
	InitializeComponent();


}


void AdControl::MSAdControl_ErrorOccurred(Platform::Object^ sender, Microsoft::Advertising::WinRT::UI::AdErrorEventArgs^ e)
{
	this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
	{
		MSAdControl->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		AdDuplex::AdControl^ AdDuplexAdControl = ref new AdDuplex::AdControl();
		AdDuplexAdControl->AdUnitId = "166555";
		AdDuplexAdControl->AppKey = "53f0124b-fa9c-40be-a364-4589d505adcd";
		adGrid->Children->Append(AdDuplexAdControl);
		
	}));
}


void AdControl::MSAdControl_AdRefreshed(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
	{
		
		MSAdControl->Visibility = Windows::UI::Xaml::Visibility::Visible;

		//AdDuplexAdControl->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		//AdDuplexAdControl->IsEnabled = false;  //set to false to stop checking for ad
	}));
}


void AdControl::AdDuplexAdControl_AdCovered(Platform::Object^ sender, AdDuplex::Banners::Core::AdCoveredEventArgs^ e)
{
	String^ test = e->CulpritElement->GetType()->FullName;
	String^ test2 = e->CulpritElement->Name;
}
