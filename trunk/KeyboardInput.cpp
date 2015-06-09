#include "KeyboardInput.h"
#include "EmulatorSettings.h"
#include <Windows.h>

namespace VBA10
{
	KeyboardInput::KeyboardInput(void)
		: window(CoreWindow::GetForCurrentThread())
	{
		ZeroMemory(&state, sizeof(ControllerState));
	}

	KeyboardInput::~KeyboardInput(void)
	{
		this->window = nullptr;
	}

	const ControllerState *KeyboardInput::GetControllerState(void)
	{
		return &this->state;
	}
	
	void KeyboardInput::Update(void)
	{
		ZeroMemory(&state, sizeof(ControllerState));

		this->state.LeftPressed = (bool)(window->GetKeyState(GetLeftKeyBinding()) & CoreVirtualKeyStates::Down);
		this->state.RightPressed = (bool)(window->GetKeyState(GetRightKeyBinding()) & CoreVirtualKeyStates::Down);
		this->state.UpPressed = (bool)(window->GetKeyState(GetUpKeyBinding()) & CoreVirtualKeyStates::Down);
		this->state.DownPressed = (bool)(window->GetKeyState(GetDownKeyBinding()) & CoreVirtualKeyStates::Down);

		this->state.StartPressed = (bool)(window->GetKeyState(GetStartKeyBinding()) & CoreVirtualKeyStates::Down);
		this->state.SelectPressed = (bool)(window->GetKeyState(GetSelectKeyBinding()) & CoreVirtualKeyStates::Down);
		
		this->state.APressed = (bool)(window->GetKeyState(GetAKeyBinding()) & CoreVirtualKeyStates::Down);
		this->state.BPressed = (bool)(window->GetKeyState(GetBKeyBinding()) & CoreVirtualKeyStates::Down);
		this->state.LPressed = (bool)(window->GetKeyState(GetLKeyBinding()) & CoreVirtualKeyStates::Down);
		this->state.RPressed = (bool)(window->GetKeyState(GetRKeyBinding()) & CoreVirtualKeyStates::Down);

		this->state.TurboTogglePressed = (bool)(window->GetKeyState(GetTurboKeyBinding()) & CoreVirtualKeyStates::Down);
	}

}