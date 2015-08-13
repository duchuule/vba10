#include "HIDControllerInput.h"

using namespace Windows::Devices::HumanInterfaceDevice;

namespace VBA10
{
	HIDControllerInput::HIDControllerInput()
	{
		
	}

	HIDControllerInput::~HIDControllerInput()
	{

	}

	const ControllerState *HIDControllerInput::GetControllerState()
	{
		return &this->state;
	}

	void HIDControllerInput::Update()
	{

	}


	HidNumericControlExt::HidNumericControlExt(HidNumericControlDescription^ desc)
	{
		this->Description = desc;
	}

}