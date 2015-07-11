#ifndef NO_XBOX
#include "ControllerInput.h"

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

			this->state.LeftPressed = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) || (state.Gamepad.sThumbLX < (-XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)));
			this->state.RightPressed = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) || (state.Gamepad.sThumbLX > (XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)));
			this->state.UpPressed = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) || (state.Gamepad.sThumbLY > (XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)));
			this->state.DownPressed = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) || (state.Gamepad.sThumbLY < (-XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)));

			this->state.StartPressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START);
			this->state.SelectPressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);

			this->state.APressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B);
			this->state.BPressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
			this->state.YPressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X);
			this->state.XPressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
			this->state.LPressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
			this->state.RPressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);

			this->state.TurboPressed = ((state.Gamepad.BCenterXTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) || (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD));
			this->state.TurboTogglePressed = this->state.XPressed || this->state.YPressed;
		}else
		{
			ZeroMemory(&this->state, sizeof(ControllerState));
		}
	}

}
#endif