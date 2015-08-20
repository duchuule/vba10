#include "CXBOXController.h"
#ifndef NO_XBOX
namespace VBA10
{
	CXBOXController::CXBOXController(int playerNumber)
		: controllerNumber(playerNumber - 1)
	{ 
		
	}

	XINPUT_STATE CXBOXController::GetState(void)
	{
		ZeroMemory(&this->state, sizeof(XINPUT_STATE));
		XInputGetState(this->controllerNumber, &this->state);  //get state of controller #controlerNumber (hardcode 0 at the moment)
															   //see this->p1Controller = new ControllerInput(1);

		return this->state;
	}

	bool CXBOXController::IsConnected(void)
	{
		//ZeroMemory(&this->state, sizeof(XINPUT_STATE));
		bool connected = false;
		for (int i = 0; i <= 3; i++)
		{
			int cNumber = (this->controllerNumber + i) % 4;  //try from the current controller number first

			if (XInputGetState(cNumber, &this->state) == ERROR_SUCCESS)
			{
				controllerNumber = cNumber;
				connected = true;
				break;
			}
		}



		return connected;
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