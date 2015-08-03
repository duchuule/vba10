//
// SelectStatePane.xaml.h
// Declaration of the SelectStatePane class
//

#pragma once

#include "SelectFilePane.g.h"

using namespace Windows::Foundation::Collections;
using namespace Platform::Collections;
using namespace Windows::Storage;

namespace VBA10
{

	public delegate void FileSelectedDelegate(StorageFile ^file);

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class SelectFilePane sealed
	{
	public:
		SelectFilePane(IVectorView<StorageFile^>^ list);
		property FileSelectedDelegate ^FileSelectedCallback;
	private:
		bool initdone;

		Platform::Collections::Vector<StorageFile^>^ fileVector;

		void Init(void);

		void fileList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
	};
}
