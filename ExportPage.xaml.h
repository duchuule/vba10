//
// ExportPage.xaml.h
// Declaration of the ExportPage class
//

#pragma once

#include "ExportPage.g.h"

namespace VBA10
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class ExportPage sealed
	{
	public:
		ExportPage();
	private:
		void exportOneDrivebtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void SignInbtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void signin_Completed(bool isLoggedIn);
	};
}
