#pragma once

#include <collection.h>
#include "EmulatorInput.h"

namespace VBA10
{
	ref class HidNumericControlExt sealed
	{
	public:
		HidNumericControlExt(unsigned short usagepage, unsigned short usageid);
	internal:
		unsigned short UsagePage;
		unsigned short UsageId;
		long long DefaultValue;
		long long MaximumValue;
		Platform::String^ Assignment;
		int Type; //0: only on-off state , 1: left - off -right state  , 2: value state
		Platform::Collections::Map<int, Platform::String^>^ Mapping;

	};


	ref class HIDControllerInput sealed
	{
	public:
		HIDControllerInput();
		
	internal:
		const ControllerState* GetControllerState(void);
		void Update();

		void StartListening();  //start listening to report event
		void StopListening();
		

		//Windows::Devices::HumanInterfaceDevice::HidDevice ^Device;
		
		Platform::Collections::Vector < HidNumericControlExt^>^ allNumericControls;
		Platform::Collections::Map <int, Platform::String^>^ booleanControlMapping;
		

	private:
		ControllerState state;

		bool isListening;
		Windows::Foundation::EventRegistrationToken inputReportEventToken;
		Windows::Devices::HumanInterfaceDevice::HidInputReport^ inputReport;

		~HIDControllerInput(void);
		void OnInputReportEvent(
			Windows::Devices::HumanInterfaceDevice::HidDevice^ sender,
			Windows::Devices::HumanInterfaceDevice::HidInputReportReceivedEventArgs^ eventArgs);

		CRITICAL_SECTION inputSync;

		void GetMapping(Platform::String^ tag, bool* left, bool* right, bool* up, bool* down, bool* a, bool* b, bool* l, bool* r, bool* select, bool* start, bool* turbo);

	};


	

	
}