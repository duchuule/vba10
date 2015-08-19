#include "HIDControllerInput.h"

using namespace Windows::Devices::HumanInterfaceDevice;
using namespace Platform::Collections;
using namespace Windows::Foundation;

namespace VBA10
{
	HIDControllerInput::HIDControllerInput(): isListening(false)
	{
		InitializeCriticalSectionEx(&inputSync, NULL, NULL);

		//initialize boolean button map
		this->booleanControlMapping = ref new Map <int, Platform::String^>();
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
		//if (Device == nullptr)
		//	return;

		//if (!isListening)
		//{
		//	// Save event registration token so we can unregisted for events
		//	inputReportEventToken = Device->InputReportReceived +=
		//		ref new TypedEventHandler<HidDevice^, HidInputReportReceivedEventArgs^>(this, &HIDControllerInput::OnInputReportEvent);

		//	isListening = true;
		//}
	}

	void HIDControllerInput::StopListening()
	{
		//if (Device == nullptr)
		//	return;

		//if (isListening)
		//{
		//	Device->InputReportReceived -= inputReportEventToken;
		//	isListening = false;
		//}

	}

	void HIDControllerInput::OnInputReportEvent(HidDevice^ sender, HidInputReportReceivedEventArgs^ eventArgs)
	{


		// Store  data from the InputReport
		//EnterCriticalSection(&inputSync);
		HidInputReport^ inputReport = eventArgs->Report;
		//LeaveCriticalSection(&inputSync);

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


		//check buttons
		auto bcontrols = inputReport->ActivatedBooleanControls;

		for (int i = 0; i < bcontrols->Size; i++)
		{
			auto control = bcontrols->GetAt(0);

			int id = control->Id;

			if (booleanControlMapping->HasKey(id))
				GetMapping(booleanControlMapping->Lookup(id), &state.LeftPressed, &state.RightPressed, &state.UpPressed, &state.DownPressed,
					&state.APressed, &state.BPressed, &state.LPressed, &state.RPressed, &state.SelectPressed, &state.StartPressed, &state.TurboPressed);
		}

		//check numeric control
		for (int i = 0;i < allNumericControls->Size; i++) //loop through all available numeric control to see which one change value
		{
			auto controlExt = allNumericControls->GetAt(i);
			auto control = inputReport->GetNumericControl(controlExt->UsagePage, controlExt->UsageId);





			if (controlExt->Type == 0 && control->Value != controlExt->DefaultValue)  //trigger
			{
				if (controlExt->Mapping->HasKey(1))
					GetMapping(controlExt->Mapping->Lookup(1), &state.LeftPressed, &state.RightPressed, &state.UpPressed, &state.DownPressed,
						&state.APressed, &state.BPressed, &state.LPressed, &state.RPressed, &state.SelectPressed, &state.StartPressed, &state.TurboPressed);
			}
			else if (controlExt->Type == 1 && control->Value - controlExt->DefaultValue < -0.25 * controlExt->DefaultValue) //axis-
			{
				if (controlExt->Mapping->HasKey(-1))
					GetMapping(controlExt->Mapping->Lookup(-1), &state.LeftPressed, &state.RightPressed, &state.UpPressed, &state.DownPressed,
						&state.APressed, &state.BPressed, &state.LPressed, &state.RPressed, &state.SelectPressed, &state.StartPressed, &state.TurboPressed);
			}
			else if (controlExt->Type == 1 && control->Value - controlExt->DefaultValue > 0.25 * controlExt->DefaultValue) //axis+
			{
				if (controlExt->Mapping->HasKey(1))
					GetMapping(controlExt->Mapping->Lookup(1), &state.LeftPressed, &state.RightPressed, &state.UpPressed, &state.DownPressed,
						&state.APressed, &state.BPressed, &state.LPressed, &state.RPressed, &state.SelectPressed, &state.StartPressed, &state.TurboPressed);
			}
			else if (controlExt->Type == 2 && control->Value != controlExt->DefaultValue) //hat d-pad
			{
				if (controlExt->Mapping->HasKey(control->Value))
					GetMapping(controlExt->Mapping->Lookup(control->Value), &state.LeftPressed, &state.RightPressed, &state.UpPressed, &state.DownPressed,
						&state.APressed, &state.BPressed, &state.LPressed, &state.RPressed, &state.SelectPressed, &state.StartPressed, &state.TurboPressed);
			}
		}


	}

	void HIDControllerInput::Update()
	{
		if (this->inputReport == nullptr)
			return;


		




	}

	void HIDControllerInput::GetMapping(Platform::String^ tag, bool* left, bool* right, bool* up, bool* down, bool* a, bool* b, bool* l, bool* r, bool* select, bool* start, bool* turbo)
	{
		if (tag == "DownLeft")
		{
			*left = true;
			*down = true;
		}
		else if (tag == "UpLeft")
		{
			*left = true;
			*up = true;
		}
		else if (tag == "DownRight")
		{
			*right = true;
			*down = true;
		}
		else if (tag == "UpRight")
		{
			*right = true;
			*up = true;
		}
		else if (tag == "Left1" || tag == "Left2" )
			*left = true;
		else if (tag == "Right1" || tag == "Right2" )
			*right = true;
		else if (tag == "Up1" || tag == "Up2" )
			*up = true;
		else if (tag == "Down1" || tag == "Down2" )
			*down = true;
		else if (tag == "A1" || tag == "A2")
			*a = true;
		else if (tag == "B1" || tag == "B2")
			*b = true;
		else if (tag == "L1" || tag == "L2")
			*l = true;
		else if (tag == "R1" || tag == "R2")
			*r = true;
		else if (tag == "Select1" || tag == "Select2")
			*select = true;
		else if (tag == "Start1" || tag == "Start2")
			*start = true;
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