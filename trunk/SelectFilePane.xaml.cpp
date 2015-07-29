//
// SelectStatePane.xaml.cpp
// Implementation of the SelectStatePane class
//

#include "pch.h"
#include "SelectFilePane.xaml.h"
#include "DirectXPage.xaml.h";

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

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236


SelectFilePane::SelectFilePane(IVectorView<StorageFile^>^ list):initdone(false)
{
	InitializeComponent();
	this->fileVector = ref new Vector<StorageFile^>();

	
	for (int i = 0; i < list->Size; i++)
		this->fileVector->Append(list->GetAt(i));

	this->FileListvs->Source = this->fileVector;
	this->fileList->SelectedItem = nullptr;

	if (list->Size > 0)
	{
		this->fileList->Visibility = Windows::UI::Xaml::Visibility::Visible;
		this->txtNoFile->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	}
	else
	{
		this->fileList->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		this->txtNoFile->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}
	initdone = true;
}


void SelectFilePane::fileList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (initdone)
	{
		StorageFile ^file = (StorageFile ^)(this->fileList->SelectedItem);

		//close the pane
		auto dp = this->Parent;
		Popup^ pop = (Popup^)dp;
		pop->IsOpen = false;

		//return the file to whatever windows that call it
		if (this->FileSelectedCallback)
			FileSelectedCallback(file);

		
	}
}
