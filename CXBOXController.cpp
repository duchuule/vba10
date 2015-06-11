#include "CXBOXController.h"
#ifndef NO_XBOX
namespace VBA10
{
	CXBOXController::CXBOXController(int playerNumber)
		: controllerNumber(playerNumber - 1)
	{ }

	XINPUT_STATE CXBOXController::GetState(void)
	{
		ZeroMemory(&this->state, sizeof(XINPUT_STATE));
		XInputGetState(this->controllerNumber, &this->state);

		return this->state;
	}

	bool CXBOXController::IsConnected(void)
	{
		//ZeroMemory(&this->state, sizeof(XINPUT_STATE));

		DWORD result = XInputGetState(this->controllerNumber, &this->state);

		return (result == ERROR_SUCCESS);
	}

	void CXBOXController::Vibrate(int leftVal, int rightVal)
	{
		XINPUT_VIBRATION vibration;
		ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

		vibration.wLeftMotorSpeed = leftVal;
		vibration.wRightMotorSpeed = rightVal;

		XInputSetState(this->controllerNumber, &vibration);
	}
}
#endif