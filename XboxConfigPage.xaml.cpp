//
// XboxConfigPage.xaml.cpp
// Implementation of the XboxConfigPage class
//

#include "pch.h"
#include "XboxConfigPage.xaml.h"
#include "EmulatorSettings.h"

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

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

XboxConfigPage::XboxConfigPage()
{
	InitializeComponent();

	auto loader = Windows::ApplicationModel::Resources::ResourceLoader::GetForViewIndependentUse();
	
	//set value for combo box
	Vector<String^>^ appFunctionList = ref new Vector<String^> { "None", "A", "B", "L", "R", "A + B", loader->GetString("TurboText") };
	cboA->ItemsSource = appFunctionList;
	cboB->ItemsSource = appFunctionList;
	cboX->ItemsSource = appFunctionList;
	cboY->ItemsSource = appFunctionList;
	cboL1->ItemsSource = appFunctionList;
	cboL2->ItemsSource = appFunctionList;
	cboR1->ItemsSource = appFunctionList;
	cboR2->ItemsSource = appFunctionList;
	cboL3->ItemsSource = appFunctionList;
	cboR3->ItemsSource = appFunctionList;

	cboA->SelectedIndex = EmulatorSettings::Current->XboxA;
	cboB->SelectedIndex = EmulatorSettings::Current->XboxB;
	cboX->SelectedIndex = EmulatorSettings::Current->XboxX;
	cboY->SelectedIndex = EmulatorSettings::Current->XboxY;
	cboL1->SelectedIndex = EmulatorSettings::Current->XboxL1;
	cboL2->SelectedIndex = EmulatorSettings::Current->XboxL2;
	cboR1->SelectedIndex = EmulatorSettings::Current->XboxR1;
	cboR2->SelectedIndex = EmulatorSettings::Current->XboxR2;
	cboL3->SelectedIndex = EmulatorSettings::Current->XboxL3;
	cboR3->SelectedIndex = EmulatorSettings::Current->XboxR3;
	

}


void XboxConfigPage::CancelBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (Frame->CanGoBack)
	{
		Frame->GoBack();
	}
}


void XboxConfigPage::OkBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//save settings
	EmulatorSettings::Current->XboxA = cboA->SelectedIndex;
	EmulatorSettings::Current->XboxB = cboB->SelectedIndex;
	EmulatorSettings::Current->XboxX = cboX->SelectedIndex;
	EmulatorSettings::Current->XboxY = cboY->SelectedIndex;
	EmulatorSettings::Current->XboxL1 = cboL1->SelectedIndex;
	EmulatorSettings::Current->XboxL2 = cboL2->SelectedIndex;
	EmulatorSettings::Current->XboxR1 = cboR1->SelectedIndex;
	EmulatorSettings::Current->XboxR2 = cboR2->SelectedIndex;
	EmulatorSettings::Current->XboxL3 = cboL3->SelectedIndex;
	EmulatorSettings::Current->XboxR3 = cboR3->SelectedIndex;

	if (Frame->CanGoBack)
	{
		Frame->GoBack();
	}
}
