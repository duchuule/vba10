#pragma once

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

	ref class HidNumericControlExt sealed
	{
	public:
		HidNumericControlExt(Windows::Devices::HumanInterfaceDevice::HidNumericControlDescription^ desc);
		property Windows::Devices::HumanInterfaceDevice::HidNumericControlDescription^ Description;
		property long long DefaultValue;
	};
}