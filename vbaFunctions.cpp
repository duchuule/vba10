#include "vbam/System.h"
#include "VirtualControllerInput.h"
#include "KeyboardInput.h"
#include "ControllerInput.h"
#include "Emulator.h"
#include "EmulatorSettings.h"

using namespace VBA10;

bool enableTurboMode = false;

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

u32 systemReadJoypad(int gamepad) 
{ 
	u32 res = 0;

	EmulatorGame *emulator = EmulatorGame::GetInstance();
	KeyboardInput *keyboard = emulator->GetKeyboardInput();
	ControllerInput *controller = emulator->GetControllerInput();

	VirtualControllerInput *vController = VirtualControllerInput::GetInstance();
	if(!controller || !vController || !keyboard)
		return res;

	//const ControllerState *state = controller->GetControllerState();
	const ControllerState *keyboardState = keyboard->GetControllerState();
	const ControllerState *controllerState = controller->GetControllerState();
	const ControllerState *vControllerState = vController->GetControllerState();

	if(keyboardState->APressed || controllerState->APressed || vControllerState->APressed)
		res |= 1;
	if(keyboardState->BPressed || controllerState->BPressed || vControllerState->BPressed)
		res |= 2;
	if(keyboardState->SelectPressed || controllerState->SelectPressed || vControllerState->SelectPressed)
		res |= 4;
	if(keyboardState->StartPressed || controllerState->StartPressed || vControllerState->StartPressed)
		res |= 8;
	if(keyboardState->RightPressed || controllerState->RightPressed || vControllerState->RightPressed)
		res |= 16;
	if(keyboardState->LeftPressed || controllerState->LeftPressed || vControllerState->LeftPressed)
		res |= 32;
	if(keyboardState->UpPressed || controllerState->UpPressed || vControllerState->UpPressed)
		res |= 64;
	if(keyboardState->DownPressed || controllerState->DownPressed || vControllerState->DownPressed)
		res |= 128;
	if(keyboardState->RPressed || controllerState->RPressed || vControllerState->RPressed)
		res |= 256;
	if(keyboardState->LPressed || controllerState->LPressed || vControllerState->LPressed)
		res |= 512;

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
			enableTurboMode = !enableTurboMode;
			toggledThisUpdate = true;
		}
	}
	if(!toggledThisUpdate)
	{
		if(!keyboardState->TurboTogglePressed && oldKeyboardState.TurboTogglePressed)
		{
			enableTurboMode = !enableTurboMode;
			toggledThisUpdate = true;
		}
	}
	if(!toggledThisUpdate)
	{
		if(!controllerState->TurboTogglePressed && oldControllerState.TurboTogglePressed)
		{
			enableTurboMode = !enableTurboMode;
			toggledThisUpdate = true;
		}
	}

	if(enableTurboMode &&
		(oldKeyboardState.TurboPressed && !keyboardState->TurboPressed) ||
		(oldControllerState.TurboPressed && !controllerState->TurboPressed) ||
		(oldvControllerState.TurboPressed && !vControllerState->TurboPressed))
	{
		enableTurboMode = false;
	}

	if(enableTurboMode || keyboardState->TurboPressed || controllerState->TurboPressed || vControllerState->TurboPressed)
		res |= 1024;
	
	oldControllerState = *controllerState;
	oldKeyboardState = *keyboardState;
	oldvControllerState = *vControllerState;

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