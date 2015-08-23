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
		AdDuplexAdControl->Visibility = Windows::UI::Xaml::Visibility::Visible;
		
	}));
}


void AdControl::MSAdControl_AdRefreshed(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
	{
		AdDuplexAdControl->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		MSAdControl->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}));
}
