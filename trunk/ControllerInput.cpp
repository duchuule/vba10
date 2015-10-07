#ifndef NO_XBOX
#include "ControllerInput.h"
#include "EmulatorSettings.h"

namespace VBA10
{
	ControllerInput::ControllerInput(int index)
		: xboxPad(new CXBOXController(index))
	{
		ZeroMemory(&state, sizeof(ControllerState));		
	}

	ControllerInput::~ControllerInput(void)
	{
		delete this->xboxPad;
	}

	const ControllerState *ControllerInput::GetControllerState(void)
	{
		return &this->state;
	}

	void ControllerInput::Update(void)
	{
		if(this->xboxPad->IsConnected())
		{
			

			XINPUT_STATE state = this->xboxPad->GetState();

			ZeroMemory(&this->state, sizeof(ControllerState));
			

			//buttons that do not allow reassignment
			this->state.LeftPressed = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) || (state.Gamepad.sThumbLX < (-XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)));
			this->state.RightPressed = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) || (state.Gamepad.sThumbLX >(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)));
			this->state.UpPressed = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) || (state.Gamepad.sThumbLY > (XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)));
			this->state.DownPressed = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) || (state.Gamepad.sThumbLY < (-XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)));

			this->state.StartPressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START);
			this->state.SelectPressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);

			//other buttons that allow reassignment
			bool turboButtonPressed = false;

			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_A)
				GetMapping(EmulatorSettings::Current->XboxA, &this->state.APressed, &this->state.BPressed, &this->state.LPressed,
					&this->state.RPressed, &turboButtonPressed);

			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
				GetMapping(EmulatorSettings::Current->XboxB, &this->state.APressed, &this->state.BPressed, &this->state.LPressed,
					&this->state.RPressed, &turboButtonPressed);

			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_X)
				GetMapping(EmulatorSettings::Current->XboxX, &this->state.APressed, &this->state.BPressed, &this->state.LPressed,
					&this->state.RPressed, &turboButtonPressed);

			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
				GetMapping(EmulatorSettings::Current->XboxY, &this->state.APressed, &this->state.BPressed, &this->state.LPressed,
					&this->state.RPressed, &turboButtonPressed);

			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)  //L1
				GetMapping(EmulatorSettings::Current->XboxL1, &this->state.APressed, &this->state.BPressed, &this->state.LPressed,
					&this->state.RPressed, &turboButtonPressed);

			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)  //R1
				GetMapping(EmulatorSettings::Current->XboxR1, &this->state.APressed, &this->state.BPressed, &this->state.LPressed,
					&this->state.RPressed, &turboButtonPressed);

			if (state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)  //L2
				GetMapping(EmulatorSettings::Current->XboxL2, &this->state.APressed, &this->state.BPressed, &this->state.LPressed,
					&this->state.RPressed, &turboButtonPressed);

			if (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)  //R2
				GetMapping(EmulatorSettings::Current->XboxR2, &this->state.APressed, &this->state.BPressed, &this->state.LPressed,
					&this->state.RPressed, &turboButtonPressed);

			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)  //L3
				GetMapping(EmulatorSettings::Current->XboxL3, &this->state.APressed, &this->state.BPressed, &this->state.LPressed,
					&this->state.RPressed, &turboButtonPressed);

			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)  //R3
				GetMapping(EmulatorSettings::Current->XboxR3, &this->state.APressed, &this->state.BPressed, &this->state.LPressed,
					&this->state.RPressed, &turboButtonPressed);

			if (EmulatorSettings::Current->TurboBehavior == 0)
				this->state.TurboTogglePressed = turboButtonPressed;
			else
				this->state.TurboPressed = turboButtonPressed;

		

			
			
			
		}else
		{
			ZeroMemory(&this->state, sizeof(ControllerState));
		}
	}


	void ControllerInput::GetMapping(int pressedButton, bool* a, bool* b, bool* l, bool* r, bool* turbo)
	{
		if (pressedButton == 1)
			*a = true;
		else if (pressedButton == 2)
			*b = true;
		else if (pressedButton == 3)
			*l = true;
		else if (pressedButton == 4)
			*r = true;
		else if (pressedButton == 5)
		{
			*a = true;
			*b = true;
		}
		else if (pressedButton == 6)
			*turbo = true;
	}

}
#endif