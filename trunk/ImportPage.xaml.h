//
// ImportPage.xaml.h
// Declaration of the ImportPage class
//

#pragma once

#include "ImportPage.g.h"



namespace VBA10
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class ImportPage sealed
	{
	public:
		ImportPage();
		void ImportFile(Windows::Storage::StorageFile^ file);



	private:
		void chooseFolderbtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void importSavbtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void SignInbtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void importOneDriveROMbtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void signin_Completed(bool isLoggedIn);
	};
}
