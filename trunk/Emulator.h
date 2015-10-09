#ifndef EMULATOR_H_
#define EMULATOR_H_

#include <ppltasks.h>
#include <collection.h>
#include "VirtualControllerInput.h"
#include "KeyboardInput.h"
#include "ControllerInput.h"
#include "GameTime.h"
#include <System.h>
#include "HIDControllerInput.h"

using namespace Platform::Collections;
using namespace concurrency;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::FileProperties;
using namespace Windows::UI::Input;
using namespace Windows::System::Threading;
using namespace Windows::System;

extern CRITICAL_SECTION pauseSync;

namespace VBA10
{
	struct ROMData
	{
		unsigned char *ROM;
		size_t Length;
	};

	class EmulatorGame
	{
	public:
		static EmulatedSystem emulator;

		EmulatorGame(bool restore);
		~EmulatorGame(void);
		void Start(void);
		void ReleaseAllResources(void);
		static EmulatorGame *GetInstance();
		VirtualControllerInput *GetVirtualController(void) const;
		KeyboardInput *GetKeyboardInput(void) const;
#ifndef NO_XBOX
		ControllerInput *GetControllerInput(void) const;
#endif
		HIDControllerInput ^GetHidControllerInput(void) const;

		

		int GetWidth(void);
		int GetHeight(void);
		void FocusChanged(bool focus);
		void ResizeBuffer(float width, float height);
		void Initialize();
		task<void> StopROMAsync(void);
		bool IsPaused(void);
		void Pause(void);
		void Unpause(void);
		void Update(float timeDelta);
		bool LastFrameSkipped(void);
		void StartEmulatorThread();
		void StopEmulatorThread();
		void ResetXboxTimer();
		float GetXboxTimer();

		HIDControllerInput ^HidInput;

		bool RestoreHidConfig();

		void EnterButtonEditMode();
		bool IsButtonEditMode();
		void LeaveButtonEditMode(bool accept);

	private:
		static EmulatorGame *instance;

		uint32_t frontbuffer, backbuffer;		
		bool stopThread;
		Windows::Foundation::IAsyncAction ^threadAction;
		HANDLE updateEvent;
		HANDLE flipBufferEvent;
		HANDLE sleepEvent;	
		uint32_t width, height;
		BYTE *gfxbuffer;
		bool focus;		
		bool graphicsResourcesReleased;
		bool restoreState;
		float xboxElapsed;
#ifndef NO_XBOX
		ControllerInput *p1Controller;
#endif
		KeyboardInput *keyboard;
		VirtualControllerInput *virtualInput;
		

		int updateCount;
		bool frameSkipped;

		void UpdateAsync(void);
		void InitSound(void);
		void DeInitSound(void);
		void FlipBuffers(void *buffer, size_t rowPitch);
		
	
	};
}

#endif