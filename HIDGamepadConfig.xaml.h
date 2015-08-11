//
// HIDGamepadConfig.xaml.h
// Declaration of the HIDGamepadConfig class
//

#pragma once

#include "HIDGamepadConfig.g.h"

namespace VBA10
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class HIDGamepadConfig sealed
	{
	public:
		HIDGamepadConfig();
	private:
		void txtLeft1_GotFocus(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void closeBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
