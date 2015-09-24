//
// SettingsPage.xaml.cpp
// Implementation of the SettingsPage class
//

#include "pch.h"
#include "SettingsPage.xaml.h"
#include "EmulatorSettings.h"
#include <string>
#include <sstream>
#include <Xinput.h>
#include <HIDGamepadConfig.xaml.h>
#include "EventHandlerForDevice.h"
#include "DirectXPage.xaml.h"
#include "App.xaml.h"
#include "AdControl.xaml.h"
#include "Filter/ShaderManager.h"

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
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Popups;
using namespace Windows::Globalization;
using namespace Windows::UI::ViewManagement;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::HumanInterfaceDevice;
using namespace Microsoft::Advertising::WinRT::UI;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

SettingsPage::SettingsPage()
	: initdone(false), emulator(EmulatorGame::GetInstance())
{
	InitializeComponent();

	//create ad control
	if (App::HasAds)
	{
		AdControl^ adControl = ref new AdControl();
		LayoutRoot->Children->Append(adControl);
		adControl->SetValue(Grid::RowProperty, 2);
	}

	//notice text about watch video
	if (App::IsPremium)
	{
		panelGamepadActivate->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		this->runBuyNotice->Text = "";
	}
	else
	{
		if (emulator->GetXboxTimer() >= 3600.0f)
			this->runBuyNotice->Foreground = ref new SolidColorBrush(Windows::UI::Colors::DarkRed);
	}

	this->touchToggle->IsOn = TouchControlsEnabled();
	this->UpdateTextBox(this->leftKeyBox, GetLeftKeyBinding());
	this->UpdateTextBox(this->rightKeyBox, GetRightKeyBinding());
	this->UpdateTextBox(this->upKeyBox, GetUpKeyBinding());
	this->UpdateTextBox(this->downKeyBox, GetDownKeyBinding());
	this->UpdateTextBox(this->startKeyBox, GetStartKeyBinding());
	this->UpdateTextBox(this->selectKeyBox, GetSelectKeyBinding());
	this->UpdateTextBox(this->aKeyBox, GetAKeyBinding());
	this->UpdateTextBox(this->bKeyBox, GetBKeyBinding());
	this->UpdateTextBox(this->lKeyBox, GetLKeyBinding());
	this->UpdateTextBox(this->rKeyBox, GetRKeyBinding());
	this->UpdateTextBox(this->turboModeBox, GetTurboKeyBinding());
	this->controllerScaleSlider->Value = (double)EmulatorSettings::Current->ControllerScale;
	this->buttonScaleSlider->Value = (double)EmulatorSettings::Current->ButtonScale;
	this->controllerOpacitySlider->Value = (double)GetControllerOpacity();
	this->deadzoneSlider->Value = (double)GetDeadzone();
	this->dpadComboBox->SelectedIndex = EmulatorSettings::Current->DPadStyle;
	this->cboTurboBehavior->SelectedIndex = EmulatorSettings::Current->TurboBehavior;

	//change the settings that depend on enabletouchcontrol
	touchToggle_Toggled(nullptr, nullptr);

	//video
	this->enableTurboToggle->IsOn = EmulatorSettings::Current->EnableTurbo;
	this->skipComboBox->SelectedIndex = (EmulatorSettings::Current->FrameSkip + 1 < this->skipComboBox->Items->Size) ? (EmulatorSettings::Current->FrameSkip + 1) : (this->skipComboBox->Items->Size - 1);
	this->turboSkipComboBox->SelectedIndex = (EmulatorSettings::Current->TurboFrameSkip - 1 < this->turboSkipComboBox->Items->Size) ? (EmulatorSettings::Current->TurboFrameSkip - 1) : (this->turboSkipComboBox->Items->Size - 1);
	this->monitorComboBox->SelectedIndex = GetMonitorType();
	this->fpsToggle->IsOn = ShowingFPS();
	switch (GetAspectRatio())
	{
	default:
	case AspectRatioMode::Original:
		this->aspectComboBox->SelectedIndex = 0;
		break;
	case AspectRatioMode::Stretch:
		this->aspectComboBox->SelectedIndex = 1;
		break;
	case AspectRatioMode::FourToThree:
		this->aspectComboBox->SelectedIndex = 2;
		break;
	case AspectRatioMode::FiveToFour:
		this->aspectComboBox->SelectedIndex = 3;
		break;
	case AspectRatioMode::One:
		this->aspectComboBox->SelectedIndex = 4;
		break;
	}
	this->cboPixelFilter->SelectedIndex = EmulatorSettings::Current->PixelFilter;
	this->cboPixelShader->SelectedIndex = EmulatorSettings::Current->PixelShader;
	//this->linearFilterToggle->IsOn = EmulatorSettings::Current->LinearFilterEnabled;
	this->fullscreenToggle->IsOn = EmulatorSettings::Current->FullScreen;

	//general
	this->loadConfirmationToggle->IsOn = IsLoadConfirmationDisabled();
	this->saveConfirmationToggle->IsOn = IsSaveConfirmationDisabled();
	this->soundToggle->IsOn = SoundEnabled();
	this->soundSyncToggle->IsOn = SynchronizeAudio();
	this->cboTheme->SelectedIndex = EmulatorSettings::Current->Theme;

	//hamburger
	if (Windows::Foundation::Metadata::ApiInformation::IsTypePresent("Windows.Phone.UI.Input.HardwareButtons"))
	{
		this->hideHamburgerToggle->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}
	this->hideHamburgerToggle->IsOn = EmulatorSettings::Current->HideHamburger;

	//check xbox controller connection
	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));
	bool xboxConnected = false;

	for (int i = 0; i <= 3; i++)
	{
		if (XInputGetState(i, &state) != ERROR_DEVICE_NOT_CONNECTED)
		{
			xboxConnected = true;
			break;
		}
	}

	if (!xboxConnected)
	{
		this->txtControllerStatus->Text = "No XBox controller detected.";
	}
	else
	{
		this->txtControllerStatus->Text = "XBox controller is connected.";
	}

	
	//check hid gamepad connection
	auto deviceSelector = HidDevice::GetDeviceSelector(0x0001, 0x0005);
	create_task(DeviceInformation::FindAllAsync(deviceSelector))
		.then([this](DeviceInformationCollection^ collection)
	{
		
			//VID_045E = microsoft
			this->HIDDeviceList = ref new Vector<DeviceInformation^>();
			Vector<String^>^ deviceIDs = ref new Vector<String^>();
			for (int i = 0; i < collection->Size; i++)
			{
				DeviceInformation^ device = collection->GetAt(i);
				
				//ignore microsoft xbox controller
				wstring deviceid(device->Id->Begin(), device->Id->End());
				if (deviceid.find(L"VID_045E") != string::npos)
					continue;

				this->HIDDeviceList->Append(device);
				deviceIDs->Append(device->Name);
			}

		if (this->HIDDeviceList->Size > 0)
		{
			this->vsControllerList->Source = deviceIDs;
			this->lbHIDGamepad->SelectedItem = nullptr;
			this->lbHIDGamepad->Visibility = Windows::UI::Xaml::Visibility::Visible;
			this->panelHIDConnect->Visibility = Windows::UI::Xaml::Visibility::Visible;

			if (EventHandlerForDevice::Current->IsDeviceConnected)
				this->txtHIDGamepad->Text = EventHandlerForDevice::Current->DeviceInformation->Name + " is connected.";
			else
				this->txtHIDGamepad->Text = this->HIDDeviceList->Size + " HID gamepad(s) detected:";
		}
		else
		{
			this->txtHIDGamepad->Text = "No HID gamepad detected.";
			this->lbHIDGamepad->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
			this->panelHIDConnect->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		}

	}, task_continuation_context::use_current());

	
	initdone = true;
}


void SettingsPage::purchaseBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	DirectXPage::Current->GoToPage(4);
}

void SettingsPage::watchVideobtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//reset xbox controller timer

	

	this->MyVideoAd = ref new InterstitialAd();

	MyVideoAd->AdReady += ref new Windows::Foundation::EventHandler<Platform::Object ^>(this, &SettingsPage::OnAdReady);
	MyVideoAd->ErrorOccurred += ref new Windows::Foundation::EventHandler<Microsoft::Advertising::WinRT::UI::AdErrorEventArgs ^>(this, &VBA10::SettingsPage::OnErrorOccurred);
	MyVideoAd->Cancelled += ref new Windows::Foundation::EventHandler<Platform::Object ^>(this, &VBA10::SettingsPage::OnCancelled);
	MyVideoAd->Completed += ref new Windows::Foundation::EventHandler<Platform::Object ^>(this, &VBA10::SettingsPage::OnCompleted);
	//MyVideoAd->RequestAd(AdType::Video, "90156e70-3263-4775-8d1e-918443468f13", "11533084");  //mobile
	MyVideoAd->RequestAd(AdType::Video, "c6ee4c5e-e2b6-4b79-99e6-672f765f0ae0", "11533083");  //PC/tablet
	//MyVideoAd->RequestAd(AdType::Video, "d25517cb-12d4-4699-8bdc-52040c712cab", "11389925");  //TEST MODE

	
}

void SettingsPage::OnAdReady(Platform::Object ^sender, Platform::Object ^args)
{
	if (InterstitialAdState::Ready == MyVideoAd->State)
	{
		MyVideoAd->Show();

		
	}

}

void SettingsPage::OnErrorOccurred(Platform::Object ^sender, Microsoft::Advertising::WinRT::UI::AdErrorEventArgs ^args)
{
	this->emulator->ResetXboxTimer();
	this->runBuyNotice->Foreground = ref new SolidColorBrush(Windows::UI::Colors::Black);
	MessageDialog ^dialog = ref new MessageDialog("Thanks! Enjoy your Xbox controller for the next hour. Click this button again after the time expires to continue using Xbox controller.");
	dialog->ShowAsync();
}


void SettingsPage::OnCancelled(Platform::Object ^sender, Platform::Object ^args)
{
	MessageDialog ^dialog = ref new MessageDialog("You need to watch the whole video to activate the feature.");
	dialog->ShowAsync();
}

void SettingsPage::OnCompleted(Platform::Object ^sender, Platform::Object ^args)
{
	this->emulator->ResetXboxTimer();
	this->runBuyNotice->Foreground = ref new SolidColorBrush(Windows::UI::Colors::Black);
	MessageDialog ^dialog = ref new MessageDialog("Thanks! Enjoy your Xbox controller for the next hour. Click this button again after the time expires to continue using Xbox controller.");
	dialog->ShowAsync();
}

void SettingsPage::ConfigureBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (this->HIDDeviceList->Size == 0)
		return;

	int index = lbHIDGamepad->SelectedIndex;

	if (this->HIDDeviceList->Size > 1 && index < 0)
	{
		MessageDialog ^dialog = ref new MessageDialog("Please select a HID gamepad.");
		dialog->ShowAsync();
		return;
	}
	else if (this->HIDDeviceList->Size == 1)
		index = 0;


	create_task(EventHandlerForDevice::Current->OpenDeviceAsync(this->HIDDeviceList->GetAt(index)))
		.then([this](task<bool> openDeviceTask)
	{
		try
		{
			bool openSuccess = openDeviceTask.get();

			this->Frame->Navigate(
				TypeName(HIDGamepadConfig::typeid),
				nullptr,
				ref new Windows::UI::Xaml::Media::Animation::DrillInNavigationTransitionInfo());
		}
		catch (const std::exception &) {}
		catch (Exception^) {}
	});


	
}


void SettingsPage::ConnectBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	int index = lbHIDGamepad->SelectedIndex;

	if (this->HIDDeviceList->Size > 1 && index < 0)
	{
		MessageDialog ^dialog = ref new MessageDialog("Please select a HID gamepad.");
		dialog->ShowAsync();
		return;
	}
	else if (this->HIDDeviceList->Size == 1)
		index = 0;

	create_task(EventHandlerForDevice::Current->OpenDeviceAsync(this->HIDDeviceList->GetAt(index)))
		.then([this, index](task<bool> openDeviceTask)
	{
		try
		{
			bool openSuccess = openDeviceTask.get();

			if (openSuccess)
			{
				this->txtHIDGamepad->Text = EventHandlerForDevice::Current->DeviceInformation->Name + " is connected.";

				create_task(emulator->RestoreHidConfig())
					.then([this](bool restoreSuccess)
				{
					if (!restoreSuccess)
					{
						//open dialog
						MessageDialog ^dialog = ref new MessageDialog("Looks like this is the first time you connect this gamepad. Click OK to configure it.");

						UICommand ^confirm = ref new UICommand("OK",
							ref new UICommandInvokedHandler([this](IUICommand ^cmd)
						{
							this->Frame->Navigate(
								TypeName(HIDGamepadConfig::typeid),
								nullptr,
								ref new Windows::UI::Xaml::Media::Animation::DrillInNavigationTransitionInfo());
						}));

						dialog->Commands->Append(confirm);
						dialog->ShowAsync();
					}
				});
			}
			else
			{
				this->txtHIDGamepad->Text = "Failed to connect to " + this->HIDDeviceList->GetAt(index)->Name;
			}
		}
		catch (const std::exception &) {}
		catch (Exception^) {}


	}, task_continuation_context::use_current());
}

void SettingsPage::cboTurboBehavior_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (initdone)
	{
		EmulatorSettings::Current->TurboBehavior = this->cboTurboBehavior->SelectedIndex;
	}
}

void SettingsPage::touchToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (initdone)
	{
		EnableTouchControls(this->touchToggle->IsOn);
	}
	//this->controllerScaleSlider->IsEnabled = this->touchToggle->IsOn;
	//this->buttonScaleSlider->IsEnabled = this->touchToggle->IsOn;
	//this->controllerOpacitySlider->IsEnabled = this->touchToggle->IsOn;
	//this->deadzoneSlider->IsEnabled = this->touchToggle->IsOn;
	this->dpadComboBox->IsEnabled = this->touchToggle->IsOn;
	this->editButonLayoutBtn->IsEnabled = this->touchToggle->IsOn;
}

void SettingsPage::UpdateTextBox(Windows::UI::Xaml::Controls::TextBox ^box, VirtualKey vk)
{
	auto s = vk.ToString();
	if (s->Length() >= 20)
	{
		s = "Unknown";
	}

	box->Text = s;
}


void SettingsPage::keyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	e->Handled = true;
}

void SettingsPage::leftKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->leftKeyBox, e->Key);
	SetLeftKeyBinding(e->Key);
}


void SettingsPage::upKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->upKeyBox, e->Key);
	SetUpKeyBinding(e->Key);
}


void SettingsPage::rightKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->rightKeyBox, e->Key);
	SetRightKeyBinding(e->Key);
}


void SettingsPage::downKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->downKeyBox, e->Key);
	SetDownKeyBinding(e->Key);
}


void SettingsPage::bKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->bKeyBox, e->Key);
	SetBKeyBinding(e->Key);
}


void SettingsPage::aKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->aKeyBox, e->Key);
	SetAKeyBinding(e->Key);
}


void SettingsPage::lKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->lKeyBox, e->Key);
	SetLKeyBinding(e->Key);
}


void SettingsPage::rKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->rKeyBox, e->Key);
	SetRKeyBinding(e->Key);
}


void SettingsPage::startKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->startKeyBox, e->Key);
	SetStartKeyBinding(e->Key);
}


void SettingsPage::selectKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->selectKeyBox, e->Key);
	SetSelectKeyBinding(e->Key);
}


void SettingsPage::turboModeBox_KeyUp_1(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	this->UpdateTextBox(this->turboModeBox, e->Key);
	SetTurboKeyBinding(e->Key);
}




void SettingsPage::controllerScaleSlider_ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	//if (this->valueLabel)
	//{
	//	wstringstream wss;
	//	wss << (int)e->NewValue;

	//	this->valueLabel->Text = ref new String(wss.str().c_str());

		if (initdone)
		{
			EmulatorSettings::Current->ControllerScale = (int)e->NewValue;
		}
	//}
}


void SettingsPage::buttonScaleSlider_ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	//if (this->valueLabel2)
	//{
	//	wstringstream wss;
	//	wss << (int)e->NewValue;

	//	this->valueLabel2->Text = ref new String(wss.str().c_str());

		if (initdone)
		{
			EmulatorSettings::Current->ButtonScale = (int)e->NewValue;
		}
	//}
}

void SettingsPage::controllerOpacitySlider_ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	//if (this->opacityValueLabel)
	//{
	//	wstringstream wss;
	//	wss << (int)e->NewValue;

	//	this->opacityValueLabel->Text = ref new String(wss.str().c_str());

		if (initdone)
		{
			SetControllerOpacity((int)e->NewValue);
		}
	//}
}


void SettingsPage::dpadComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (this->initdone)
	{
		EmulatorSettings::Current->DPadStyle = this->dpadComboBox->SelectedIndex;
	}
}


void SettingsPage::deadzoneSlider_ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	//if (this->deadzoneLabel)
	//{
	//	wstringstream wss;
	//	wss << (((int)(e->NewValue * 10.0)) / 10.0f);

	//	this->deadzoneLabel->Text = ref new Platform::String(wss.str().c_str());

		if (initdone)
		{
			SetDeadzone((float)e->NewValue);
		}
	//}
}

void SettingsPage::editButonLayoutBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	DirectXPage::Current->EditButtonLayout();
}

//###########VIDEO SETTINGS 

void SettingsPage::monitorComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (initdone)
	{
		SetMonitorType(this->monitorComboBox->SelectedIndex);
	}
}



void SettingsPage::skipComboBox_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (initdone)
	{
		EmulatorSettings::Current->FrameSkip = this->skipComboBox->SelectedIndex - 1;
	}
}


void SettingsPage::turboSkipComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (initdone)
	{
		EmulatorSettings::Current->TurboFrameSkip = this->turboSkipComboBox->SelectedIndex + 1;
	}
}



void SettingsPage::aspectComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (initdone)
	{
		AspectRatioMode mode = AspectRatioMode::Original;
		switch (this->aspectComboBox->SelectedIndex)
		{
		case 0:
			mode = AspectRatioMode::Original;
			break;
		case 1:
			mode = AspectRatioMode::Stretch;
			break;
		case 2:
			mode = AspectRatioMode::FourToThree;
			break;
		case 3:
			mode = AspectRatioMode::FiveToFour;
			break;
		case 4:
			mode = AspectRatioMode::One;
		}
		SetAspectRatio(mode);
	}
}

void SettingsPage::cboPixelFilter_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (initdone)
	{

		EmulatorSettings::Current->PixelFilter = this->cboPixelFilter->SelectedIndex;

		if (!App::IsPremium)
		{
			MessageDialog ^dialog = ref new MessageDialog("This is a premium feature. You can use the feature now but the setting will revert to None the next time the app starts.");
			dialog->ShowAsync();
		}
	}
}


void SettingsPage::cboPixelShader_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (initdone)
	{

		EmulatorSettings::Current->PixelShader = this->cboPixelShader->SelectedIndex;

		if (this->cboPixelShader->SelectedIndex != 0)
			ShaderManager::GetInstance()->LoadShader(this->cboPixelShader->SelectedIndex);
		//if (!App::IsPremium)
		//{
		//	MessageDialog ^dialog = ref new MessageDialog("This is a premium feature. You can use the feature now but the setting will revert to None the next time the app starts.");
		//	dialog->ShowAsync();
		//}
	}
}

//void SettingsPage::linearFilterToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
//{
//	if (initdone)
//	{
//		EmulatorSettings::Current->LinearFilterEnabled = this->linearFilterToggle->IsOn;
//	}
//}

void SettingsPage::fpsToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (initdone)
	{
		ShowFPS(this->fpsToggle->IsOn);
	}
}


//####### SOUND SETTING
void SettingsPage::saveConfirmationToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (this->initdone)
	{
		DisableSaveConfirmation(this->saveConfirmationToggle->IsOn);
	}
}


void SettingsPage::loadConfirmationToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (this->initdone)
	{
		DisableLoadConfirmation(this->loadConfirmationToggle->IsOn);
	}
}

void SettingsPage::soundToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (initdone)
	{
		EnableSound(this->soundToggle->IsOn);
	}
}


void SettingsPage::soundSyncToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (initdone)
	{
		SetSynchronizeAudio(this->soundSyncToggle->IsOn);
	}
}







void SettingsPage::enableTurboToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (initdone)
	{
		EmulatorSettings::Current->EnableTurbo = this->enableTurboToggle->IsOn;
	}
}


void SettingsPage::fullscreenToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (initdone)
	{
		//save setting
		EmulatorSettings::Current->FullScreen = this->fullscreenToggle->IsOn;

		//try enter/exit full screenmode
		ApplicationView^ view = ApplicationView::GetForCurrentView();


		if (this->fullscreenToggle->IsOn)
		{
			view->TryEnterFullScreenMode();
			view->PreferredLaunchWindowingMode = ApplicationViewWindowingMode::FullScreen;
		}
		else
		{
			view->ExitFullScreenMode();
			view->PreferredLaunchWindowingMode = ApplicationViewWindowingMode::Auto;
		}
	}
}



void SettingsPage::cboTheme_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (initdone)
	{
		if (this->cboTheme->SelectedIndex != EmulatorSettings::Current->Theme)
		{
			EmulatorSettings::Current->Theme = this->cboTheme->SelectedIndex;
			MessageDialog ^dialog = ref new MessageDialog("The theme will be applied the next time the app starts.");
			dialog->ShowAsync();
		}
	}
}





void SettingsPage::hideHamburgerToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (initdone)
	{
		EmulatorSettings::Current->HideHamburger = this->hideHamburgerToggle->IsOn;
	}
}





