#include "HIDControllerInput.h"

using namespace Windows::Devices::HumanInterfaceDevice;
using namespace Platform::Collections;

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


	NumericControlMapping::NumericControlMapping(long long minval, long long maxval, Platform::String^ assignment)
	{
		this->MinVal = minval;
		this->MaxVal = maxval;
		this->Assignment = assignment;
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