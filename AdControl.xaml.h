//
// AdControl.xaml.h
// Declaration of the AdControl class
//

#pragma once

#include "AdControl.g.h"

namespace VBA10
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class AdControl sealed
	{
	public:
		AdControl();
	private:
		//void MSAdControl_ErrorOccurred(Platform::Object^ sender, Microsoft::Advertising::WinRT::UI::AdErrorEventArgs^ e);
		//void MSAdControl_AdRefreshed(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		//void AdDuplexAdControl_AdCovered(Platform::Object^ sender, AdDuplex::Banners::Core::AdCoveredEventArgs^ e);
	};
}
