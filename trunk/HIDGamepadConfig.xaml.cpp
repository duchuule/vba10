//
// HIDGamepadConfig.xaml.cpp
// Implementation of the HIDGamepadConfig class
//

#include "pch.h"
#include "HIDGamepadConfig.xaml.h"
#include "Emulator.h"
#include <robuffer.h>
#include <math.h>
#include <Windows.h>

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
using namespace Windows::Devices::HumanInterfaceDevice;
using namespace Windows::Storage::Streams;
using namespace Microsoft::WRL;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

HIDGamepadConfig::HIDGamepadConfig() :isRegisteredForInputReportEvents(false), navigatedAway(false), configureStage(0)
{
	InitializeComponent();


	registeredDevice = EmulatorGame::GetInstance()->GetHIDDevice();
	if (registeredDevice == nullptr) //connection failed
	{
		//this->txtNotification->Visibility = Windows::UI::Xaml::Visibility::Visible;
		//this->gridMain->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		this->txtNotification->Text = "Error, device connection failed.";
	}
	else
	{
		//this->txtNotification->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		//this->gridMain->Visibility = Windows::UI::Xaml::Visibility::Visible;
		this->txtNotification->Text = "First press Start button.";

		//create list of numeric controls
		this->allNumericControls = ref new Vector < HidNumericControlExt^>();
		for (unsigned short usagePage = 1; usagePage <= 5; usagePage++)
		{
			for (unsigned short usageId = 1; usageId < 255; usageId++)
			{


				auto numDescs = registeredDevice->GetNumericControlDescriptions(HidReportType::Input, usagePage, usageId);
				
				if (numDescs->Size == 1)  //only this is the real control
				{
					HidNumericControlExt^ control = ref new HidNumericControlExt( numDescs->GetAt(0));
					this->allNumericControls->Append(control);
				}
			}
		}


		if (!isRegisteredForInputReportEvents)
		{
			// Save event registration token so we can unregisted for events
			inputReportEventToken = registeredDevice->InputReportReceived +=
				ref new TypedEventHandler<HidDevice^, HidInputReportReceivedEventArgs^>(this, &HIDGamepadConfig::OnInputReportEvent);

			isRegisteredForInputReportEvents = true;
		}
	}
	
}

void HIDGamepadConfig::OnInputReportEvent(HidDevice^ sender, HidInputReportReceivedEventArgs^ eventArgs)
{
	// The data from the InputReport
	HidInputReport^ inputReport = eventArgs->Report;


	//check buttons
	auto bcontrols = inputReport->ActivatedBooleanControls;


	if (configureStage == 0) //record start button
	{
		//get default value of numeric button
		for (int i = 0;i < allNumericControls->Size; i++)
		{
			auto controlExt = allNumericControls->GetAt(i);
			auto control = inputReport->GetNumericControlByDescription(controlExt->Description);
			controlExt->DefaultValue = control->Value;

			//fix default value for xbox controller axis (potentially other high precision controller as well)
			if (24576 < controlExt->DefaultValue && controlExt->DefaultValue < 40960)
				controlExt->DefaultValue = 32768;
		}


		for (int i = 0; i < bcontrols->Size; i++)
		{
			auto item = bcontrols->GetAt(0);

			startbuttonID = item->Id;
			configureStage = 2;

		

			this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, item]()
			{
				txtNotification->Text = "You can now assign buttons as you wish. Up to two buttons can be assigned for each function.";
				this->gridMain->Visibility = Windows::UI::Xaml::Visibility::Visible;
			}));

			break;  //only take the first button press as id of start button
					
		}
	}
	//else if (configureStage == 1) //record select button
	//{
	//	for (int i = 0; i < bcontrols->Size; i++)
	//	{
	//		auto item = bcontrols->GetAt(0);

	//		if (item->Id != startbuttonID)
	//		{
	//			selectbuttonID = item->Id;
	//			configureStage = 2;
	//			this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, item]()
	//			{
	//				txtNotification->Text = "You can now assign buttons as you wish. Up to two buttons can be assigned for each function.";
	//				this->gridMain->Visibility = Windows::UI::Xaml::Visibility::Visible;
	//			}));
	//			break;  //only take the first button press as id of start button
	//		}
	//	}
	//}
	else  //free stage
	{
		if (focusTextbox != nullptr)
		{
			for (int i = 0; i < bcontrols->Size; i++)
			{
				auto control = bcontrols->GetAt(0);

				int id = control->Id;
				bool isActive = control->IsActive;
				int usagePage = control->UsagePage;
				int usageId = control->UsageId;



				this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control]()
				{
					focusTextbox->Text = "Button " + control->Id.ToString();
				}));
			}


			for (int i = 0;i < allNumericControls->Size; i++)
			{
				auto controlExt = allNumericControls->GetAt(i);
				auto control = inputReport->GetNumericControlByDescription(controlExt->Description);
				if (controlExt->DefaultValue <= 15 && control->Value != controlExt->DefaultValue)  //this is the d-pad
				{
					long long value = control->Value;
					this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control, value]()
					{
						//note: we include value in the capture list because control->Value can change back to default value
						focusTextbox->Text = "Button (" + control->UsagePage.ToString() + "," + control->UsageId.ToString() + ") - " + value;
					}));
				}
				else if (controlExt->DefaultValue > 15 && control->Value - controlExt->DefaultValue < -0.25 * controlExt->DefaultValue)
				{
					this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control]()
					{
						focusTextbox->Text = "Button (" + control->UsagePage.ToString() + "," + control->UsageId.ToString() + ") -";
					}));
				}
				else if (controlExt->DefaultValue > 15 && control->Value - controlExt->DefaultValue > 0.25 * controlExt->DefaultValue)
				{
					this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control]()
					{
						focusTextbox->Text = "Button (" + control->UsagePage.ToString() + "," + control->UsageId.ToString() + ") +";
					}));
				}

			}
		}
		//check numeric controls
		auto hat = inputReport->GetNumericControl(0x01, 0x39);
		int min = hat->ControlDescription->LogicalMinimum;
		int max = hat->ControlDescription->LogicalMaximum;
		int value = hat->Value;

		//check joystick
		auto joystick = inputReport->GetNumericControl(0x01, 48);
		min = joystick->ControlDescription->PhysicalMinimum;
		max = joystick->ControlDescription->PhysicalMaximum;
		value = joystick->ScaledValue;
	}


		


}


void HIDGamepadConfig::txtLeft1_GotFocus(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto textbox = (TextBox^)sender;

	focusTextbox = textbox;
	currentFocus = textbox->Name;

//#if _DEBUG
//	Platform::String ^message = textbox->Name + " got focus\n";
//	wstring wstr(message->Begin(), message->End());
//	OutputDebugStringW(wstr.c_str());
//#endif

}





void HIDGamepadConfig::txtLeft1_LostFocus(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto textbox = (TextBox^)sender;

	focusTextbox = nullptr;
	currentFocus = "";
}

void HIDGamepadConfig::OnNavigatedFrom(NavigationEventArgs^ /* e */)
{
	navigatedAway = true;

	if (isRegisteredForInputReportEvents)
	{
		// Don't unregister event token if the device was removed and reconnected because registration token is no longer valid
		registeredDevice->InputReportReceived -= inputReportEventToken;
		registeredDevice = nullptr;
		isRegisteredForInputReportEvents = false;
	}


}


void HIDGamepadConfig::closeBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (Frame->CanGoBack)
	{
		Frame->GoBack();
	}
}