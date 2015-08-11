//
// HIDGamepadConfig.xaml.cpp
// Implementation of the HIDGamepadConfig class
//

#include "pch.h"
#include "HIDGamepadConfig.xaml.h"

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

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

HIDGamepadConfig::HIDGamepadConfig()
{
	InitializeComponent();
}


void HIDGamepadConfig::txtLeft1_GotFocus(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void HIDGamepadConfig::closeBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (Frame->CanGoBack)
	{
		Frame->GoBack();
	}
}
