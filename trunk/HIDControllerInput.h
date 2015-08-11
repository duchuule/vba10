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

	private:
		ControllerState state;
		Windows::Devices::HumanInterfaceDevice::HidDevice ^device;
	};
}