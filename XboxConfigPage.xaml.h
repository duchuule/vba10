//
// XboxConfigPage.xaml.h
// Declaration of the XboxConfigPage class
//

#pragma once

#include "XboxConfigPage.g.h"

namespace VBA10
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class XboxConfigPage sealed
	{
	public:
		XboxConfigPage();
	private:
		void CancelBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OkBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
