//
// SelectFilesPane.xaml.cpp
// Implementation of the SelectFilesPane class
//

#include "pch.h"
#include "SelectFilesPane.xaml.h"

using namespace VBA10;

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Storage;
using namespace Windows::UI::Popups;
using namespace Windows::ApplicationModel::Resources;
// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

SelectFilesPane::SelectFilesPane(IVector<Platform::String^>^ list, Platform::String^ title) :initdone(false)
{
	InitializeComponent();

	this->txtTitle->Text = title;

	this->FileListvs->Source = list;
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
		this->txtNoFile->Text = ResourceLoader::GetForViewIndependentUse()->GetString("NoSupportedFileText");
	}
	initdone = true;

}

void SelectFilesPane::CancelBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//close the pane
	auto dp = this->Parent;
	Popup^ pop = (Popup^)dp;
	pop->IsOpen = false;
}

void SelectFilesPane::OkBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (this->fileList->SelectedItems->Size == 0)
	{
		MessageDialog ^dialog = ref new MessageDialog(ResourceLoader::GetForViewIndependentUse()->GetString("NoFileSelectedText"));
		dialog->ShowAsync();
		return;
	}

	this->fileList->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	this->txtNoFile->Visibility = Windows::UI::Xaml::Visibility::Visible;
	this->OkBtn->IsEnabled = false;
	this->txtNoFile->Text = ResourceLoader::GetForViewIndependentUse()->GetString("ProcessingText");


	Vector<int>^ selectedIndices = ref new Vector<int>  ();

	for (int i = 0; i < this->fileList->SelectedItems->Size; i++)
	{
		String^ selectedName = (String^)this->fileList->SelectedItems->GetAt(i);
		//find the index of this item
		for (int j = 0; j < this->fileList->Items->Size; j++)
		{
			String^ name = (String^)this->fileList->Items->GetAt(j);
			if (selectedName == name)
			{
				selectedIndices->Append(j);
				break;
			}
		}
	}




	//return the file to whatever windows that call it
	if (this->FilesSelectedCallback)
		FilesSelectedCallback(selectedIndices);
}



