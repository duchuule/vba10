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
	auto loader = Windows::ApplicationModel::Resources::ResourceLoader::GetForViewIndependentUse();
	if (!EventHandlerForDevice::Current->IsDeviceConnected) //connection failed
	{
		//this->txtNotification->Visibility = Windows::UI::Xaml::Visibility::Visible;
		//this->gridMain->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		this->txtNotification->Text = loader->GetString("DeviceConnectionFailText");
	}
	else
	{
		//this->txtNotification->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		//this->gridMain->Visibility = Windows::UI::Xaml::Visibility::Visible;
		this->txtNotification->Text = loader->GetString("PressStartPrompt");




		//Read stored configuration and display
		create_task(emulator->RestoreHidConfig())
			.then([this, loader] (bool success) 
		{
			RegisterForInputReportEvents();

			EventHandlerForDevice::Current->OnDeviceConnected =
				ref new TypedEventHandler<EventHandlerForDevice^, OnDeviceConnectedEventArgs^>(this, &HIDGamepadConfig::OnDeviceConnected);

			EventHandlerForDevice::Current->OnDeviceClose =
				ref new TypedEventHandler<EventHandlerForDevice^, DeviceInformation^>(this, &HIDGamepadConfig::OnDeviceClosing);


			if (success)
			{

				//display boolean control
				for (auto bpair : emulator->HidInput->booleanControlMapping)
				{
					int bid = bpair->Key;
					String^ function = bpair->Value;

					TextBox^ txtbox = FindTextbox(function);

					if (txtbox != nullptr)
						txtbox->Text = loader->GetString("ButtonText") + " " + bid.ToString();
				}

				//display numeric control
				for (auto ncontrol : emulator->HidInput->allNumericControls)
				{
					if (ncontrol->UsagePage == 0x01 && ncontrol->UsageId == 0x39)  //text for hat switch has already been printed
					{

						hasHatSwitch = true;
						txtLeft1->Text = loader->GetString("DpadLeftText");
						txtUp1->Text = loader->GetString("DpadUpText");
						txtRight1->Text = loader->GetString("DpadRightText");
						txtDown1->Text = loader->GetString("DpadDownText");
						txtLeft1->IsEnabled = false;
						txtUp1->IsEnabled = false;
						txtRight1->IsEnabled = false;
						txtDown1->IsEnabled = false;

						continue;
					}

					for (auto npair : ncontrol->Mapping)
					{
						int nid = npair->Key;
						String^ function = npair->Value;

						TextBox^ txtbox = FindTextbox(function);

						if (txtbox != nullptr)
						{
							if (ncontrol->Type == 0)
								txtbox->Text = loader->GetString("AxisText") + " (" + ncontrol->UsagePage.ToString() + "," + ncontrol->UsageId.ToString() + ")";
							else if (ncontrol->Type == 1)
							{
								if (nid == -1)
									txtbox->Text = loader->GetString("AxisText") + " (" + ncontrol->UsagePage.ToString() + "," + ncontrol->UsageId.ToString() + ") -";
								else if (nid == 1)
									txtbox->Text = loader->GetString("AxisText") + " (" + ncontrol->UsagePage.ToString() + "," + ncontrol->UsageId.ToString() + ") +";
							}

						}
					}
				}
			}
			else //does not find stored configuration for this device
			{
				//get the list of numeric control
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
								txtLeft1->Text = loader->GetString("DpadLeftText");
								txtUp1->Text = loader->GetString("DpadUpText");
								txtRight1->Text = loader->GetString("DpadRightText");
								txtDown1->Text = loader->GetString("DpadDownText");
								txtLeft1->IsEnabled = false;
								txtUp1->IsEnabled = false;
								txtRight1->IsEnabled = false;
								txtDown1->IsEnabled = false;
							}
						}
					}
				}
			}
		}, task_continuation_context::use_current());


		
	}


}

TextBox^ HIDGamepadConfig::FindTextbox(Platform::String^ tag)
{
	if (tag == "Left1")
		return this->txtLeft1;
	else if (tag == "Left2")
		return this->txtLeft2;
	else if (tag == "Right1")
		return this->txtRight1;
	else if (tag == "Right2")
		return this->txtRight2;
	else if (tag == "Up1")
		return this->txtUp1;
	else if (tag == "Up2")
		return this->txtUp2;
	else if (tag == "Down1")
		return this->txtDown1;
	else if (tag == "Down2")
		return this->txtDown2;
	else if (tag == "A1")
		return this->txtA1;
	else if (tag == "A2")
		return this->txtA2;
	else if (tag == "B1")
		return this->txtB1;
	else if (tag == "B2")
		return this->txtB2;
	else if (tag == "L1")
		return this->txtL1;
	else if (tag == "L2")
		return this->txtL2;
	else if (tag == "R1")
		return this->txtR1;
	else if (tag == "R2")
		return this->txtR2;
	else if (tag == "Select1")
		return this->txtSelect1;
	else if (tag == "Select2")
		return this->txtSelect2;
	else if (tag == "Start1")
		return this->txtStart1;
	else if (tag == "Start2")
		return this->txtStart2;
	else if (tag == "Turbo1")
		return this->txtTurbo1;
	else if (tag == "Turbo2")
		return this->txtTurbo2;
	else
		return nullptr;
}



void HIDGamepadConfig::OnInputReportEvent(HidDevice^ sender, HidInputReportReceivedEventArgs^ eventArgs)
{
	// The data from the InputReport
	HidInputReport^ inputReport = eventArgs->Report;

	//check buttons
	auto bcontrols = inputReport->ActivatedBooleanControls;

	auto loader = Windows::ApplicationModel::Resources::ResourceLoader::GetForViewIndependentUse();

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

		

			this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, item, loader]()
			{
				Platform::String^ instruction = loader->GetString("HIDConfigInstruction");
				if (hasHatSwitch)
					instruction += " " + loader->GetString("DpadNote");
				txtNotification->Text = instruction;
				
				txtStart1->Text = loader->GetString("ButtonText") + " " + startbuttonID.ToString();
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



				this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control, loader]()
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

					//empty text box if this button value has been assigned to a different function
					for (auto pair : emulator->HidInput->booleanControlMapping)
					{
						auto key = pair->Key;
						auto value = pair->Value;

						if (key == control->Id && (String^)focusTextbox->Tag != value)
						{
							TextBox^ txtbox = FindTextbox(value);

							if (txtbox != nullptr)
								txtbox->Text = "";

							break;
						}
					}

					emulator->HidInput->booleanControlMapping->Insert(control->Id, (String^)focusTextbox->Tag);
					focusTextbox->Text = loader->GetString("ButtonText") + " " + control->Id.ToString();
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
					
					this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control, controlExt, loader]()
					{
						focusTextbox->Text = loader->GetString("AxisText") + " (" + control->UsagePage.ToString() + "," + control->UsageId.ToString() + ")";


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

						//empty text box if this button value has been assigned to a different function
						for (auto pair : controlExt->Mapping)
						{
							auto key = pair->Key;
							auto value = pair->Value;

							if (key == 1 && (String^)focusTextbox->Tag != value)
							{
								TextBox^ txtbox = FindTextbox(value);

								if (txtbox != nullptr)
									txtbox->Text = "";

								break;
							}
						}

						
						controlExt->Mapping->Insert(1, (String^)focusTextbox->Tag);

					}));
				}
				else if (controlExt->Type == 1 && control->Value - controlExt->DefaultValue < -0.25 * controlExt->DefaultValue)
				{
					

					this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control, controlExt, loader]()
					{
						focusTextbox->Text = loader->GetString("AxisText") + " (" + control->UsagePage.ToString() + "," + control->UsageId.ToString() + ") -";
						
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

						//empty text box if this button value has been assigned to a different function
						for (auto pair : controlExt->Mapping)
						{
							auto key = pair->Key;
							auto value = pair->Value;

							if (key == -1 && (String^)focusTextbox->Tag != value)
							{
								TextBox^ txtbox = FindTextbox(value);

								if (txtbox != nullptr)
									txtbox->Text = "";

								break;
							}
						}

						controlExt->Mapping->Insert(-1, (String^)focusTextbox->Tag);

					}));
				}
				else if (controlExt->Type == 1 && control->Value - controlExt->DefaultValue > 0.25 * controlExt->DefaultValue)
				{
					

					this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control, controlExt, loader]()
					{
						focusTextbox->Text = loader->GetString("AxisText") + " (" + control->UsagePage.ToString() + "," + control->UsageId.ToString() + ") +";
						
						//remove the function if it has been asiggned to a different button value
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

						//empty text box if this button value has been assigned to a different function
						for (auto pair : controlExt->Mapping)
						{
							auto key = pair->Key;
							auto value = pair->Value;

							if (key == 1 && (String^)focusTextbox->Tag != value)
							{
								TextBox^ txtbox = FindTextbox(value);

								if (txtbox != nullptr)
									txtbox->Text = "";

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

