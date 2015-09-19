#include "vbam/System.h"
#include "VirtualControllerInput.h"
#include "KeyboardInput.h"
#include "ControllerInput.h"
#include "Emulator.h"
#include "EmulatorSettings.h"
#include "HIDControllerInput.h"

using namespace VBA10;


void log(const char *,...) { }

bool systemPauseOnFrame() { return false; }
void systemGbPrint(u8 *,int,int,int,int) { }
void systemScreenCapture(int) { }
// updates the joystick data
bool systemReadJoypads() { return true; }
u32 systemGetClock() { return (u32) GetTickCount64(); }
void systemMessage(int, const char *, ...) { }
void systemSetTitle(const char *) { }
void systemWriteDataToSoundBuffer() { }
void systemSoundShutdown() { }
void systemSoundPause() { }
void systemSoundResume() { }
void systemSoundReset() { }
//SoundDriver *systemSoundInit() { return NULL; }
void systemScreenMessage(const char *) { }
void systemUpdateMotionSensor() { }
int  systemGetSensorX() { return 0; }
int  systemGetSensorY() { return 0; }
bool systemCanChangeSoundQuality() { return false; }
void systemShowSpeed(int){ }
void system10Frames(int){ }
void systemFrame(){ }
void systemGbBorderOn(){ }
void winlog(const char *, ...) { }
void systemOnWriteDataToSoundBuffer(const u16 * finalWave, int length) { }
void systemOnSoundShutdown() { }
extern SoundDriver *newXAudio2_Output();
extern void soundShutdown();
void systemGbPrint(unsigned char *, int, int, int, int, int) { }

SoundDriver * systemSoundInit()
{
	SoundDriver * drv = 0;
	soundShutdown();

	if(SoundEnabled())
	{
		drv = newXAudio2_Output();
	}

	return drv;
}

ControllerState oldKeyboardState;
ControllerState oldControllerState;
ControllerState oldvControllerState;
ControllerState oldHidState;

long long AUnpressedTime = 9999999999;
long long BUnpressedTime = 9999999999;
long long LUnpressedTime = 9999999999;
long long RUnpressedTime = 9999999999;
long long LeftUnpressedTime = 9999999999;
long long RightUnpressedTime = 9999999999;
long long UpUnpressedTime = 9999999999;
long long DownUnpressedTime = 9999999999;

u32 systemReadJoypad(int gamepad) 
{ 
	u32 res = 0;

	EmulatorGame *emulator = EmulatorGame::GetInstance();
	KeyboardInput *keyboard = emulator->GetKeyboardInput();
#ifndef NO_XBOX
	ControllerInput *controller = emulator->GetControllerInput();
#endif
	
	HIDControllerInput ^hidController = emulator->GetHidControllerInput();

	VirtualControllerInput *vController = VirtualControllerInput::GetInstance();
	if (
#ifndef NO_XBOX
		!controller || 
#endif 
		!vController || !keyboard)
		return res;

	//const ControllerState *state = controller->GetControllerState();
	const ControllerState *keyboardState = keyboard->GetControllerState();
#ifndef NO_XBOX
	const ControllerState *controllerState = controller->GetControllerState();
#else
	
	ControllerState controllerState1 = { false, false, false, false, false, false, false, false, false, false, false, false, false, false };
	const ControllerState *controllerState = &controllerState1;
#endif

	const ControllerState *hidState = hidController->GetControllerState();

	const ControllerState *vControllerState = vController->GetControllerState();

	int smoothButton = EmulatorSettings::Current->SmoothButton;

	if(keyboardState->APressed || controllerState->APressed || vControllerState->APressed || hidState->APressed)
	{
		res |= 1;
		AUnpressedTime = 0; //reset to 0
	}
	else
	{
		if (AUnpressedTime < smoothButton) //if not pressed for less than 3 frames
			res |= 1;

		AUnpressedTime++;
	}

	if (keyboardState->BPressed || controllerState->BPressed || vControllerState->BPressed || hidState->BPressed)
	{
		res |= 2;
		BUnpressedTime = 0; //reset to 0
	}
	else
	{
		if (BUnpressedTime < smoothButton) //if not pressed for less than 3 frames
			res |= 2;

		BUnpressedTime++;
	}

	if(keyboardState->SelectPressed || controllerState->SelectPressed || vControllerState->SelectPressed || hidState->SelectPressed)
		res |= 4;
	if(keyboardState->StartPressed || controllerState->StartPressed || vControllerState->StartPressed || hidState->StartPressed)
		res |= 8;

	if(keyboardState->RightPressed || controllerState->RightPressed || vControllerState->RightPressed || hidState->RightPressed)
	{
		res |= 16;
		RightUnpressedTime = 0; //reset to 0
	}
	else
	{
		if (RightUnpressedTime < smoothButton) //if not pressed for less than 3 frames
			res |= 16;

		RightUnpressedTime++;
	}

	if(keyboardState->LeftPressed || controllerState->LeftPressed || vControllerState->LeftPressed || hidState->LeftPressed)
	{
		res |= 32;
		LeftUnpressedTime = 0; //reset to 0
	}
	else
	{
		if (LeftUnpressedTime < smoothButton) //if not pressed for less than 3 frames
			res |= 32;

		LeftUnpressedTime++;
	}

	if(keyboardState->UpPressed || controllerState->UpPressed || vControllerState->UpPressed || hidState->UpPressed)
	{
		res |= 64;
		UpUnpressedTime = 0; //reset to 0
	}
	else
	{
		if (UpUnpressedTime < smoothButton) //if not pressed for less than 3 frames
			res |= 64;

		UpUnpressedTime++;
	}

	if(keyboardState->DownPressed || controllerState->DownPressed || vControllerState->DownPressed || hidState->DownPressed)
	{
		res |= 128;
		DownUnpressedTime = 0; //reset to 0
	}
	else
	{
		if (DownUnpressedTime < smoothButton) //if not pressed for less than 3 frames
			res |= 128;

		DownUnpressedTime++;
	}

	if(keyboardState->RPressed || controllerState->RPressed || vControllerState->RPressed || hidState->RPressed)
	{
		res |= 256;
		RUnpressedTime = 0; //reset to 0
	}
	else
	{
		if (RUnpressedTime < smoothButton) //if not pressed for less than 3 frames
			res |= 256;

		RUnpressedTime++;
	}

	if(keyboardState->LPressed || controllerState->LPressed || vControllerState->LPressed || hidState->LPressed)
	{
		res |= 512;
		LUnpressedTime = 0; //reset to 0
	}
	else
	{
		if (LUnpressedTime < smoothButton) //if not pressed for less than 3 frames
			res |= 512;

		LUnpressedTime++;
	}

	// disallow L+R or U+D of being pressed at the same time
	if((res & 48) == 48)
		res &= ~16;
	if((res & 192) == 192)
		res &= ~128;

	// Speed
	bool toggledThisUpdate = false;
	if(!toggledThisUpdate)
	{
		if(!vControllerState->TurboTogglePressed && oldvControllerState.TurboTogglePressed)
		{
			EmulatorSettings::Current->EnableTurbo = !EmulatorSettings::Current->EnableTurbo;
			toggledThisUpdate = true;
		}
	}
	if(!toggledThisUpdate)
	{
		if(!keyboardState->TurboTogglePressed && oldKeyboardState.TurboTogglePressed)
		{
			EmulatorSettings::Current->EnableTurbo = !EmulatorSettings::Current->EnableTurbo;
			toggledThisUpdate = true;
		}
	}
	if(!toggledThisUpdate)
	{
		if(!controllerState->TurboTogglePressed && oldControllerState.TurboTogglePressed)
		{
			EmulatorSettings::Current->EnableTurbo = !EmulatorSettings::Current->EnableTurbo;
			toggledThisUpdate = true;
		}
	}
	if (!toggledThisUpdate)
	{
		if (!hidState->TurboTogglePressed && oldHidState.TurboTogglePressed)
		{
			EmulatorSettings::Current->EnableTurbo = !EmulatorSettings::Current->EnableTurbo;
			toggledThisUpdate = true;
		}
	}

	//if(EmulatorSettings::Current->EnableTurbo &&
	//	(oldKeyboardState.TurboPressed && !keyboardState->TurboPressed) ||
	//	(oldControllerState.TurboPressed && !controllerState->TurboPressed) ||
	//	(oldvControllerState.TurboPressed && !vControllerState->TurboPressed) ||
	//	(oldHidState.TurboPressed && !hidState->TurboPressed))
	//{
	//	EmulatorSettings::Current->EnableTurbo = false;
	//}

	bool useTurbo = EmulatorSettings::Current->EnableTurbo;

	//reverse function if turbopressed
	if (keyboardState->TurboPressed || controllerState->TurboPressed || vControllerState->TurboPressed || hidState->TurboPressed)
		useTurbo = !useTurbo;

	if(useTurbo )
		res |= 1024;
	
	oldControllerState = *controllerState;
	oldKeyboardState = *keyboardState;
	oldvControllerState = *vControllerState;
	oldHidState = *hidState;

	return res;
}

int RGB_LOW_BITS_MASK = 65793;
int emulating;
bool systemSoundOn;
u16 systemColorMap16[0x10000];
u32 systemColorMap32[0x10000];
u16 systemGbPalette[24];
int systemRedShift;
int systemGreenShift;
int systemBlueShift;
int systemColorDepth;
int systemDebug;
int systemVerbose;
int systemFrameSkip;
int systemSaveUpdateCounter;