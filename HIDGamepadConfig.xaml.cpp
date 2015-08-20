//
// HIDGamepadConfig.xaml.cpp
// Implementation of the HIDGamepadConfig class
//

#include "pch.h"
#include "HIDGamepadConfig.xaml.h"
#include "EventHandlerForDevice.h"
#include "EmulatorFileHandler.h"

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
using namespace Windows::Devices::Enumeration;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

HIDGamepadConfig::HIDGamepadConfig() :
	isRegisteredForInputReportEvents(false), configureStage(0), hasHatSwitch(false), emulator(EmulatorGame::GetInstance())
{
	InitializeComponent();


	

	
}


void HIDGamepadConfig::OnNavigatedTo(NavigationEventArgs^ /* e */)
{
	navigatedAway = false;
	if (!EventHandlerForDevice::Current->IsDeviceConnected) //connection failed
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

		//initialize boolean button map
		emulator->HidInput->booleanControlMapping = ref new Map <int, Platform::String^ >();

		//create list of numeric controls
		emulator->HidInput->allNumericControls = ref new Vector < HidNumericControlExt^>();
		for (unsigned short usagePage = 1; usagePage <= 5; usagePage++)
		{
			for (unsigned short usageId = 1; usageId < 255; usageId++)
			{


				auto numDescs = EventHandlerForDevice::Current->Device->GetNumericControlDescriptions(HidReportType::Input, usagePage, usageId);

				if (numDescs->Size == 1)  //only this is the real control
				{
					HidNumericControlExt^ control = ref new HidNumericControlExt(usagePage, usageId);
					emulator->HidInput->allNumericControls->Append(control);

					if (usagePage == 0x01 && usageId == 0x39)  //this gamepad has hatswitch
					{
						hasHatSwitch = true;
						txtLeft1->Text = "D-pad Left";
						txtUp1->Text = "D-pad Up";
						txtRight1->Text = "D-pad Right";
						txtDown1->Text = "D-pad Down";
						txtLeft1->IsEnabled = false;
						txtUp1->IsEnabled = false;
						txtRight1->IsEnabled = false;
						txtDown1->IsEnabled = false;
					}
				}
			}
		}

		RegisterForInputReportEvents();

		EventHandlerForDevice::Current->OnDeviceConnected = 
			ref new TypedEventHandler<EventHandlerForDevice^, OnDeviceConnectedEventArgs^>(this, &HIDGamepadConfig::OnDeviceConnected);

		EventHandlerForDevice::Current->OnDeviceClose =
			ref new TypedEventHandler<EventHandlerForDevice^, DeviceInformation^>(this, &HIDGamepadConfig::OnDeviceClosing);
	}


}



void HIDGamepadConfig::OnInputReportEvent(HidDevice^ sender, HidInputReportReceivedEventArgs^ eventArgs)
{
	// The data from the InputReport
	HidInputReport^ inputReport = eventArgs->Report;

	//check buttons
	auto bcontrols = inputReport->ActivatedBooleanControls;


	if (configureStage == 0) //record start button and default value of numeric buttons
	{
		//get default value of numeric button
		for (int i = 0;i < emulator->HidInput->allNumericControls->Size; i++)
		{
			auto controlExt = emulator->HidInput->allNumericControls->GetAt(i);
			auto control = inputReport->GetNumericControl(controlExt->UsagePage, controlExt->UsageId);
			controlExt->DefaultValue = control->Value;

			//fix default value for xbox controller axis (potentially other high precision controller as well)
			if (24576 < controlExt->DefaultValue && controlExt->DefaultValue < 40960)
				controlExt->DefaultValue = 32768;
		}


		//get id of start button
		for (int i = 0; i < bcontrols->Size; i++)
		{
			auto item = bcontrols->GetAt(0);

			startbuttonID = item->Id;
			configureStage = 2;

		

			this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, item]()
			{
				Platform::String^ instruction = "You can now assign buttons as you wish. Up to two buttons can be assigned for each function.";
				if (hasHatSwitch)
					instruction += " NOTE: the D-pad can only be used for direction and cannot be assigned to other functions.";
				txtNotification->Text = instruction;
				
				txtStart1->Text = "Button " + startbuttonID.ToString();
				emulator->HidInput->booleanControlMapping->Insert( startbuttonID, "Start1");

				this->gridMain->Visibility = Windows::UI::Xaml::Visibility::Visible;
			}));

			break;  //only take the first button press as id of start button
					
		}

		//create mapping for hat switch based on default value
		if (hasHatSwitch)
		{
			//find the hat switch
			HidNumericControlExt^ controlExt;
			for (int i = 0;i < emulator->HidInput->allNumericControls->Size; i++) //loop through all available numeric control
			{
				controlExt = emulator->HidInput->allNumericControls->GetAt(i);
				if (controlExt->UsagePage == 0x01 && controlExt->UsageId == 0x39)
					break;
			}

			//set mapping
			controlExt->Type = 2;
			int offset = 1;
			if (controlExt->DefaultValue != 0)  //if default value is not zero, up has value of 0
				offset = 0;
			controlExt->Mapping->Insert(0 + offset, "Up1");
			controlExt->Mapping->Insert(1 + offset, "UpRight");
			controlExt->Mapping->Insert(2 + offset, "Right1");
			controlExt->Mapping->Insert(3 + offset, "DownRight");
			controlExt->Mapping->Insert(4 + offset, "Down1");
			controlExt->Mapping->Insert(5 + offset, "DownLeft");
			controlExt->Mapping->Insert(6 + offset, "Left1");
			controlExt->Mapping->Insert(7 + offset, "UpLeft");
		}

		//set button on ui
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

			//buttons
			for (int i = 0; i < bcontrols->Size; i++)
			{
				auto control = bcontrols->GetAt(0);

				int id = control->Id;
				bool isActive = control->IsActive;
				int usagePage = control->UsagePage;
				int usageId = control->UsageId;



				this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control]()
				{
					//remove the tag if it has been asiggned to a different button
					for (auto pair : emulator->HidInput->booleanControlMapping)
					{
						auto key = pair->Key;
						auto value = pair->Value;

						if (value == (String^)focusTextbox->Tag)
						{
							emulator->HidInput->booleanControlMapping->Remove(key);
							break;
						}
					}

					emulator->HidInput->booleanControlMapping->Insert(control->Id, (String^)focusTextbox->Tag);
					focusTextbox->Text = "Button " + control->Id.ToString();
				}));
			}

			//numeric controls
			for (int i = 0;i < emulator->HidInput->allNumericControls->Size; i++) //loop through all available numeric control to see which one change value
			{
				
				auto controlExt = emulator->HidInput->allNumericControls->GetAt(i);
				auto control = inputReport->GetNumericControl(controlExt->UsagePage, controlExt->UsageId);

				//record maximum value from input
				if (control->Value > controlExt->MaximumValue)
				{
					controlExt->MaximumValue = control->Value;
				}

				//determine button type
				if (controlExt->UsagePage == 0x01 && controlExt->UsageId == 0x39)  //d-pad
					controlExt->Type = 2;
				else if (controlExt->DefaultValue == 0)  //trigger
					controlExt->Type = 0; 
				else  //axis
					controlExt->Type = 1; 


				//if (controlExt->DefaultValue <= 15 && control->Value != controlExt->DefaultValue)  //this is the d-pad
				//{
				//	long long value = control->Value;
				//	this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control, value]()
				//	{
				//		//note: we include value in the capture list because control->Value can change back to default value
				//		focusTextbox->Text = "Button (" + control->UsagePage.ToString() + "," + control->UsageId.ToString() + ") - " + value;
				//	}));
				//}
				if (controlExt->Type == 0 && control->Value != controlExt->DefaultValue)
				{
					
					this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control, controlExt]()
					{
						focusTextbox->Text = "Button (" + control->UsagePage.ToString() + "," + control->UsageId.ToString() + ")";


						//remove the tag if it has been asiggned to a different button value
						for (auto pair : controlExt->Mapping)
						{
							auto key = pair->Key;
							auto value = pair->Value;

							if (value == (String^)focusTextbox->Tag)
							{
								controlExt->Mapping->Remove(key);
								break;
							}
						}

						
						controlExt->Mapping->Insert(1, (String^)focusTextbox->Tag);

					}));
				}
				else if (controlExt->Type == 1 && control->Value - controlExt->DefaultValue < -0.25 * controlExt->DefaultValue)
				{
					

					this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control, controlExt]()
					{
						focusTextbox->Text = "Button (" + control->UsagePage.ToString() + "," + control->UsageId.ToString() + ") -";
						
						//remove the tag if it has been asiggned to a different button value
						for (auto pair : controlExt->Mapping)
						{
							auto key = pair->Key;
							auto value = pair->Value;

							if (value == (String^)focusTextbox->Tag)
							{
								controlExt->Mapping->Remove(key);
								break;
							}
						}

						String^ test = (String^)focusTextbox->Tag;
						controlExt->Mapping->Insert(-1, (String^)focusTextbox->Tag);

					}));
				}
				else if (controlExt->Type == 1 && control->Value - controlExt->DefaultValue > 0.25 * controlExt->DefaultValue)
				{
					

					this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control, controlExt]()
					{
						focusTextbox->Text = "Button (" + control->UsagePage.ToString() + "," + control->UsageId.ToString() + ") +";
						
						//remove the tag if it has been asiggned to a different button value
						for (auto pair : controlExt->Mapping)
						{
							auto key = pair->Key;
							auto value = pair->Value;

							if (value == (String^)focusTextbox->Tag)
							{
								controlExt->Mapping->Remove(key);
								break;
							}
						}

						controlExt->Mapping->Insert(1, (String^)focusTextbox->Tag);
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

	UnregisterFromInputReportEvent();

	EventHandlerForDevice::Current->OnDeviceClose = nullptr;
	EventHandlerForDevice::Current->OnDeviceConnected = nullptr;

	//print out config
	if (EventHandlerForDevice::Current->Device != nullptr)
	{
		hidConfigs->Insert(EventHandlerForDevice::Current->DeviceInformation->Id, emulator->HidInput);
		SaveHidConfig();
	}
}



void HIDGamepadConfig::closeBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (Frame->CanGoBack)
	{
		Frame->GoBack();
	}
}

void HIDGamepadConfig::RegisterForInputReportEvents()
{
	if (!isRegisteredForInputReportEvents)
	{
		// Remember which device we are registering the device with, in case there is a device disconnect and reconnect. We want to avoid unregistering
		// a stale token. Ideally, one should remove the event token (e.g. assign to null) upon the device removal to avoid using it again.
		registeredDevice = EventHandlerForDevice::Current->Device;

		// Save event registration token so we can unregisted for events
		inputReportEventToken = registeredDevice->InputReportReceived +=
			ref new TypedEventHandler<HidDevice^, HidInputReportReceivedEventArgs^>(this, &HIDGamepadConfig::OnInputReportEvent);

		isRegisteredForInputReportEvents = true;
	}
}

void HIDGamepadConfig::UnregisterFromInputReportEvent(void)
{
	if (isRegisteredForInputReportEvents)
	{
		// Don't unregister event token if the device was removed and reconnected because registration token is no longer valid
		registeredDevice->InputReportReceived -= inputReportEventToken;
		registeredDevice = nullptr;
		isRegisteredForInputReportEvents = false;
	}
}


void HIDGamepadConfig::OnDeviceConnected(EventHandlerForDevice^ /* sender */, OnDeviceConnectedEventArgs^ onDeviceConnectedEventArgs)
{
	RegisterForInputReportEvents();
}
void HIDGamepadConfig::OnDeviceClosing(EventHandlerForDevice^ /* sender */, DeviceInformation^ /* deviceInformation */)
{
	UnregisterFromInputReportEvent();

}

