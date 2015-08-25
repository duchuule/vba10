#include "HIDControllerInput.h"

using namespace Windows::Devices::HumanInterfaceDevice;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Devices::Enumeration;

namespace VBA10
{
	HIDControllerInput::HIDControllerInput(): isRegisteredForInputReportEvents(false)
	{

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
		if (EventHandlerForDevice::Current->IsDeviceConnected)
		{
			RegisterForInputReportEvents();

			EventHandlerForDevice::Current->OnDeviceConnected =
				ref new TypedEventHandler<EventHandlerForDevice^, OnDeviceConnectedEventArgs^>(this, &HIDControllerInput::OnDeviceConnected);

			EventHandlerForDevice::Current->OnDeviceClose =
				ref new TypedEventHandler<EventHandlerForDevice^, DeviceInformation^>(this, &HIDControllerInput::OnDeviceClosing);
		}
	}

	void HIDControllerInput::StopListening()
	{
		UnregisterFromInputReportEvent();

		EventHandlerForDevice::Current->OnDeviceClose = nullptr;
		EventHandlerForDevice::Current->OnDeviceConnected = nullptr;

	}

	void HIDControllerInput::RegisterForInputReportEvents()
	{
		if (!isRegisteredForInputReportEvents)
		{
			// Remember which device we are registering the device with, in case there is a device disconnect and reconnect. We want to avoid unregistering
			// a stale token. Ideally, one should remove the event token (e.g. assign to null) upon the device removal to avoid using it again.
			registeredDevice = EventHandlerForDevice::Current->Device;

			if (registeredDevice == nullptr)
				return;

			// Save event registration token so we can unregisted for events
			inputReportEventToken = registeredDevice->InputReportReceived +=
				ref new TypedEventHandler<HidDevice^, HidInputReportReceivedEventArgs^>(this, &HIDControllerInput::OnInputReportEvent);

			isRegisteredForInputReportEvents = true;
		}
	}

	void HIDControllerInput::UnregisterFromInputReportEvent(void)
	{
		if (registeredDevice == nullptr)
			return;

		if (isRegisteredForInputReportEvents)
		{
			// Don't unregister event token if the device was removed and reconnected because registration token is no longer valid
			registeredDevice->InputReportReceived -= inputReportEventToken;
			registeredDevice = nullptr;
			isRegisteredForInputReportEvents = false;
		}
	}

	void HIDControllerInput::OnDeviceConnected(EventHandlerForDevice^ /* sender */, OnDeviceConnectedEventArgs^ onDeviceConnectedEventArgs)
	{
		RegisterForInputReportEvents();
	}
	void HIDControllerInput::OnDeviceClosing(EventHandlerForDevice^ /* sender */, DeviceInformation^ /* deviceInformation */)
	{
		UnregisterFromInputReportEvent();
	}


	void HIDControllerInput::OnInputReportEvent(HidDevice^ sender, HidInputReportReceivedEventArgs^ eventArgs)
	{


		// Store  data from the InputReport
		//EnterCriticalSection(&inputSync);
		HidInputReport^ inputReport = eventArgs->Report;
		//LeaveCriticalSection(&inputSync);

		//set all to false
		ZeroMemory(&state, sizeof(ControllerState));

		if (this->shouldUpdate == false)
			return;

		//check buttons
		auto bcontrols = inputReport->ActivatedBooleanControls;

		for (int i = 0; i < bcontrols->Size; i++)
		{
			auto control = bcontrols->GetAt(1);

			int id = control->Id;

			if (booleanControlMapping->HasKey(id))
				GetMapping(booleanControlMapping->Lookup(id), &state.LeftPressed, &state.RightPressed, &state.UpPressed, &state.DownPressed,
					&state.APressed, &state.BPressed, &state.LPressed, &state.RPressed, &state.SelectPressed, &state.StartPressed, &state.TurboTogglePressed);
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
						&state.APressed, &state.BPressed, &state.LPressed, &state.RPressed, &state.SelectPressed, &state.StartPressed, &state.TurboTogglePressed);
			}
			else if (controlExt->Type == 1 && control->Value - controlExt->DefaultValue < -0.25 * controlExt->DefaultValue) //axis-
			{
				if (controlExt->Mapping->HasKey(-1))
					GetMapping(controlExt->Mapping->Lookup(-1), &state.LeftPressed, &state.RightPressed, &state.UpPressed, &state.DownPressed,
						&state.APressed, &state.BPressed, &state.LPressed, &state.RPressed, &state.SelectPressed, &state.StartPressed, &state.TurboTogglePressed);
			}
			else if (controlExt->Type == 1 && control->Value - controlExt->DefaultValue > 0.25 * controlExt->DefaultValue) //axis+
			{
				if (controlExt->Mapping->HasKey(1))
					GetMapping(controlExt->Mapping->Lookup(1), &state.LeftPressed, &state.RightPressed, &state.UpPressed, &state.DownPressed,
						&state.APressed, &state.BPressed, &state.LPressed, &state.RPressed, &state.SelectPressed, &state.StartPressed, &state.TurboTogglePressed);
			}
			else if (controlExt->Type == 2 && control->Value != controlExt->DefaultValue) //hat d-pad
			{
				if (controlExt->Mapping->HasKey(control->Value))
					GetMapping(controlExt->Mapping->Lookup(control->Value), &state.LeftPressed, &state.RightPressed, &state.UpPressed, &state.DownPressed,
						&state.APressed, &state.BPressed, &state.LPressed, &state.RPressed, &state.SelectPressed, &state.StartPressed, &state.TurboTogglePressed);
			}
		}


	}

	void HIDControllerInput::Update(bool shouldUpdate)
	{
		this->shouldUpdate = shouldUpdate;
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