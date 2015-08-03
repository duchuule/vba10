//
// SelectROMPane.xaml.h
// Declaration of the SelectROMPane class
//

#pragma once

#include "SelectROMPane.g.h"
#include "Converter.h"
#include "Definitions.h"
#include "Database\ROMDBEntry.h"

namespace VBA10
{

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class SelectROMPane sealed
	{
	public:
		SelectROMPane();

	private:
		bool initdone;

		Platform::Collections::Vector<StorageFolderModel ^> ^storageFolderVector;
		Platform::Collections::Vector<StorageFileModel ^> ^storageFileVector;

		void ResetLastPlayedImage();

		void InitializeStorageLists(void);
		void Close(void);
		void AddRomDirectoryClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void DeleteFolderClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void RemoveFolderByToken(Platform::String ^token);
		void RefreshROMList(void);
		void romList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void saveBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void loadBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void resetBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void selectStateBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void addROMbtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		StorageFolder^ tmpFolder;
		void lastRomImage_Tapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);
		void RemoveROMButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);


		Windows::Foundation::Rect GetElementRect(Windows::UI::Xaml::FrameworkElement^ element);
		void ContextMenuBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void lbAllROMMainGrid_RightTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::RightTappedRoutedEventArgs^ e);
		void ShowContextMenu(ROMDBEntry^ entry, Windows::Foundation::Rect rect);
		void maximizebtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};


	
}
