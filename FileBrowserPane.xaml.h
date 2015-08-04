//
// FileBrowserPane.xaml.h
// Declaration of the FileBrowserPane class
//

#pragma once

#include "FileBrowserPane.g.h"
#include "Definitions.h"
#include "cpprest\json.h"


namespace VBA10
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class FileBrowserPane sealed
	{
	public:
		FileBrowserPane();

	private:
		void fileList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void closeBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		OneDriveItemType GetOneDriveItemType(std::wstring ext);
		Platform::Collections::Vector<OneDriveFileItem^>^ fileVector;
		Platform::Collections::Vector<Windows::Foundation::Collections::IVector<OneDriveFileItem^>^>^ onedriveStack;

		void client_GetCompleted(web::json::value v);
		bool loading;//keep track of when the app is loading new data
		void backBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		Concurrency::task<size_t> DownloadFile(OneDriveFileItem^ item);
		Windows::Storage::StorageFile^ tmpfile;
	};
}
