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

		this->state.LeftPressed = (bool)(window->GetAsyncKeyState(GetLeftKeyBinding()) & CoreVirtualKeyStates::Down);
		this->state.RightPressed = (bool)(window->GetAsyncKeyState(GetRightKeyBinding()) & CoreVirtualKeyStates::Down);
		this->state.UpPressed = (bool)(window->GetAsyncKeyState(GetUpKeyBinding()) & CoreVirtualKeyStates::Down);
		this->state.DownPressed = (bool)(window->GetAsyncKeyState(GetDownKeyBinding()) & CoreVirtualKeyStates::Down);

		this->state.StartPressed = (bool)(window->GetAsyncKeyState(GetStartKeyBinding()) & CoreVirtualKeyStates::Down);
		this->state.SelectPressed = (bool)(window->GetAsyncKeyState(GetSelectKeyBinding()) & CoreVirtualKeyStates::Down);
		
		this->state.APressed = (bool)(window->GetAsyncKeyState(GetAKeyBinding()) & CoreVirtualKeyStates::Down);
		this->state.BPressed = (bool)(window->GetAsyncKeyState(GetBKeyBinding()) & CoreVirtualKeyStates::Down);
		this->state.LPressed = (bool)(window->GetAsyncKeyState(GetLKeyBinding()) & CoreVirtualKeyStates::Down);
		this->state.RPressed = (bool)(window->GetAsyncKeyState(GetRKeyBinding()) & CoreVirtualKeyStates::Down);

		this->state.TurboTogglePressed = (bool)(window->GetAsyncKeyState(GetTurboKeyBinding()) & CoreVirtualKeyStates::Down);
	}

}