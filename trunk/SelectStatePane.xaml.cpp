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
	this->savestateVector = ref new Platform::Collections::Vector<Platform::String ^>();
	this->savestateVector->Append("Savestate 0");
	this->savestateVector->Append("Savestate 1");
	this->savestateVector->Append("Savestate 2");
	this->savestateVector->Append("Savestate 3");
	this->savestateVector->Append("Savestate 4");
	this->savestateVector->Append("Savestate 5");
	this->savestateVector->Append("Savestate 6");
	this->savestateVector->Append("Savestate 7");
	this->savestateVector->Append("Savestate 8");
	this->savestateVector->Append("Savestate 9 (auto)");

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
