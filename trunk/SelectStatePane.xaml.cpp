//
// SelectStatePane.xaml.cpp
// Implementation of the SelectStatePane class
//

#include "pch.h"
#include "SelectStatePane.xaml.h"
#include "DirectXPage.xaml.h"

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


SelectStatePane::SelectStatePane(int selectedSlot):initdone(false)
{
	InitializeComponent();
	this->Init();
	this->savestateList->SelectedIndex = selectedSlot;

	initdone = true;
}

void SelectStatePane::Init(void)
{
	auto loader = Windows::ApplicationModel::Resources::ResourceLoader::GetForViewIndependentUse();

	this->savestateVector = ref new Platform::Collections::Vector<Platform::String ^>();
	this->savestateVector->Append(loader->GetString("Savestate0Text"));
	this->savestateVector->Append(loader->GetString("Savestate1Text"));
	this->savestateVector->Append(loader->GetString("Savestate2Text"));
	this->savestateVector->Append(loader->GetString("Savestate3Text"));
	this->savestateVector->Append(loader->GetString("Savestate4Text"));
	this->savestateVector->Append(loader->GetString("Savestate5Text"));
	this->savestateVector->Append(loader->GetString("Savestate6Text"));
	this->savestateVector->Append(loader->GetString("Savestate7Text"));
	this->savestateVector->Append(loader->GetString("Savestate8Text"));
	this->savestateVector->Append(loader->GetString("Savestate9Text"));

	this->savestateList->ItemsSource = this->savestateVector->GetView();

	
}

void SelectStatePane::savestateList_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (initdone)
	{
		int index = this->savestateList->SelectedIndex;
		DirectXPage::Current->SelectSaveState(index);

		//close the pane
		auto dp = this->Parent;
		Popup^ pop = (Popup^)dp;
		pop->IsOpen = false;
	}
}
