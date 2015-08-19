#include "HIDControllerInput.h"

using namespace Windows::Devices::HumanInterfaceDevice;
using namespace Platform::Collections;
using namespace Windows::Foundation;

namespace VBA10
{
	HIDControllerInput::HIDControllerInput(): isListening(false)
	{
		//initialize boolean button map
		this->booleanControlMapping = ref new Map <Platform::String^, int>();
		this->allNumericControls = ref new Vector < HidNumericControlExt^>();
	}

	HIDControllerInput::~HIDControllerInput()
	{

	}

	const ControllerState *HIDControllerInput::GetControllerState()
	{
		return &this->state;
	}


	void HIDControllerInput::StartListening()
	{
		if (Device == nullptr)
			return;

		if (!isListening)
		{
			// Save event registration token so we can unregisted for events
			inputReportEventToken = Device->InputReportReceived +=
				ref new TypedEventHandler<HidDevice^, HidInputReportReceivedEventArgs^>(this, &HIDControllerInput::OnInputReportEvent);

			isListening = true;
		}
	}

	void HIDControllerInput::StopListening()
	{
		if (Device == nullptr)
			return;

		if (isListening)
		{
			Device->InputReportReceived -= inputReportEventToken;
			isListening = false;
		}

	}

	void HIDControllerInput::OnInputReportEvent(HidDevice^ sender, HidInputReportReceivedEventArgs^ eventArgs)
	{
		//set all to false
		this->state.TurboTogglePressed = false;
		this->state.StartPressed = false;
		this->state.SelectPressed = false;
		this->state.APressed = false;
		this->state.BPressed = false;
		this->state.LPressed = false;
		this->state.RPressed = false;
		this->state.LeftPressed = false;
		this->state.RightPressed = false;
		this->state.UpPressed = false;
		this->state.DownPressed = false;

		// The data from the InputReport
		HidInputReport^ inputReport = eventArgs->Report;


		//check buttons
		auto bcontrols = inputReport->ActivatedBooleanControls;

		for (int i = 0; i < bcontrols->Size; i++)
		{
			auto control = bcontrols->GetAt(0);

			int id = control->Id;

			for (auto pair: booleanControlMapping)
			{
				auto key = pair->Key;
				auto value = pair->Value;

				if (value == id)
				{
					GetMapping(key, &state.LeftPressed, &state.RightPressed, &state.UpPressed, &state.DownPressed,
						&state.APressed, &state.BPressed, &state.LPressed, &state.RPressed, &state.TurboPressed);
					break;
				}
			}
		}

		//check numeric control
		//for (int i = 0;i < emulator->HidInput->allNumericControls->Size; i++) //loop through all available numeric control to see which one change value
		//{
		//	auto controlExt = emulator->HidInput->allNumericControls->GetAt(i);
		//	auto control = inputReport->GetNumericControl(controlExt->UsagePage, controlExt->UsageId);

		//	//record maximum value from input
		//	if (control->Value > controlExt->MaximumValue)
		//	{
		//		controlExt->MaximumValue = control->Value;
		//	}

		//	//determine button type
		//	if (controlExt->UsagePage == 0x01 && controlExt->UsageId == 0x39)  //d-pad
		//		controlExt->Type = 2;
		//	else if (controlExt->DefaultValue == 0)  //trigger
		//		controlExt->Type = 0;
		//	else  //axis
		//		controlExt->Type = 1;



		//	if ((controlExt->Type == 0 && control->Value != controlExt->DefaultValue)
		//		|| 
		//	{
		//		this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control, controlExt]()
		//		{
		//			focusTextbox->Text = "Button (" + control->UsagePage.ToString() + "," + control->UsageId.ToString() + ")";
		//			controlExt->Mapping->Insert(1, (String^)focusTextbox->Tag);

		//		}));
		//	}
		//	else if (controlExt->Type == 1 && control->Value - controlExt->DefaultValue < -0.25 * controlExt->DefaultValue)
		//	{
		//		this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control, controlExt]()
		//		{
		//			focusTextbox->Text = "Button (" + control->UsagePage.ToString() + "," + control->UsageId.ToString() + ") -";
		//			String^ test = (String^)focusTextbox->Tag;
		//			controlExt->Mapping->Insert(-1, (String^)focusTextbox->Tag);

		//		}));
		//	}
		//	else if (controlExt->Type == 1 && control->Value - controlExt->DefaultValue > 0.25 * controlExt->DefaultValue)
		//	{
		//		this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, control, controlExt]()
		//		{
		//			focusTextbox->Text = "Button (" + control->UsagePage.ToString() + "," + control->UsageId.ToString() + ") +";
		//			controlExt->Mapping->Insert(1, (String^)focusTextbox->Tag);
		//		}));
		//	}
		//}
	}


	void HIDControllerInput::GetMapping(Platform::String^ tag, bool* left, bool* right, bool* up, bool* down, bool* a, bool* b, bool* l, bool* r, bool* turbo)
	{
		if (tag == "Left1" || tag == "Left2" || tag == "UpLeft" || tag == "DownLeft")
			*left = true;
		else if (tag == "Right1" || tag == "Right2" || tag == "UpRight" || tag == "DownRight")
			*right = true;
		else if (tag == "Up1" || tag == "Up2" || tag == "UpLeft" || tag == "UpRight")
			*up = true;
		else if (tag == "Down1" || tag == "Down2" || tag == "DownLeft" || tag == "DownRight")
			*down = true;
		else if (tag == "A1" || tag == "A2")
			*a = true;
		else if (tag == "B1" || tag == "B2")
			*b = true;
		else if (tag == "L1" || tag == "L2")
			*l = true;
		else if (tag == "R1" || tag == "R2")
			*r = true;
		else if (tag == "Turbo1" || tag == "Turbo2")
			*turbo = true;

	}

	HidNumericControlExt::HidNumericControlExt(unsigned short usagepage, unsigned short usageid)
	{
		this->UsagePage = usagepage;
		this->UsageId = usageid;
		this->MaximumValue = 0;
		this->Type = -1;
		this->Mapping = ref new Map<int, Platform::String^>();
	}

}