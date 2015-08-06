//
// SelectFilesPane.xaml.h
// Declaration of the SelectFilesPane class
//

#pragma once

#include "SelectFilesPane.g.h"

namespace VBA10
{
	public delegate void FilesSelectedDelegate(Windows::Foundation::Collections::IVector<int>^ selectedIndices);

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class SelectFilesPane sealed
	{
	public:
		SelectFilesPane(Windows::Foundation::Collections::IVector<Platform::String^>^ list, Platform::String^ Title);
		property FilesSelectedDelegate ^FilesSelectedCallback;
	private:
		bool initdone;
		Platform::Collections::Vector<Windows::Storage::StorageFile^>^ fileVector;
		void OkBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
