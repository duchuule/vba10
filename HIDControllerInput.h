#pragma once

#include <collection.h>
#include "EmulatorInput.h"

namespace VBA10
{
	class HIDControllerInput : public EmulatorInput
	{
	public:
		HIDControllerInput();
		~HIDControllerInput(void);

		const ControllerState *GetControllerState(void);
		void Update(void);

		Windows::Devices::HumanInterfaceDevice::HidDevice^ GetHidDevice()
		{
			return device;
		}

		void SetHidDevice(Windows::Devices::HumanInterfaceDevice::HidDevice^ device)
		{
			this->device = device;
		}

		

		

	private:
		ControllerState state;
		Windows::Devices::HumanInterfaceDevice::HidDevice ^device;
		
	};

	ref class NumericControlMapping
	{
	internal:
		NumericControlMapping(long long minval, long long maxval, Platform::String^ assignment);
		long long MinVal;
		long long MaxVal;
		Platform::String^ Assignment;
	};

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

	
}