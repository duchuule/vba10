

#ifndef CXBOXCONTROLLER_H_
#define CXBOXCONTROLLER_H_

#ifndef NO_XBOX
#include <Windows.h>
#include <Xinput.h>

namespace VBA10
{
	class CXBOXController
	{
	private:
		XINPUT_STATE state;
		int controllerNumber;

	public:
		CXBOXController (int playerNumber);
		XINPUT_STATE GetState(void);
		bool IsConnected();
		void Vibrate(int leftVal = 0, int rightVal = 0);
	};
}

#endif

#endif