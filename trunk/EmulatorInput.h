#pragma once

#include "Input.h"

namespace VBA10
{
	class EmulatorInput
	{
	public:
		virtual ~EmulatorInput(void) { }

		const virtual ControllerState *GetControllerState(void) = 0;

		virtual void Update(void) = 0;
	};
}
