//#include "App.xaml.h"
#include "Emulator.h"
#include "EmulatorFileHandler.h"
#include "EmulatorSettings.h"
#include <d3d11_1.h>
#include <GBA.h>
#include <Util.h>
#include <SoundDriver.h>

using namespace Platform;
using namespace concurrency;

using namespace Platform;
using namespace Windows::Foundation;

extern int emulating;
extern void ContinueEmulation(void);

extern SoundDriver *soundDriver;
extern long  soundSampleRate;

extern void soundSetVolume(float);

CRITICAL_SECTION pauseSync;
int turboSkip = 5;

namespace VBA10
{
	extern bool timeMeasured;
	bool autosaving;
	
	EmulatedSystem EmulatorGame::emulator = GBASystem;

	EmulatorGame *EmulatorGame::instance = nullptr;

	EmulatorGame *EmulatorGame::GetInstance(void)
	{
		return EmulatorGame::instance;
	}

	EmulatorGame::EmulatorGame(bool restore) 
		: restoreState(restore),
		graphicsResourcesReleased(false), stopThread(false),
		updateCount(0), frameSkipped(0), gfxbuffer(nullptr), threadAction(nullptr)
	{
		EmulatorGame::instance = this;
	}

	EmulatorGame::~EmulatorGame(void)
	{
		//this->threadAction->Cancel();
		this->stopThread = true;
		ContinueEmulation();
		WaitForSingleObjectEx(this->flipBufferEvent, INFINITE, false);
		this->threadAction = nullptr;

		CloseHandle(this->updateEvent);
		CloseHandle(this->flipBufferEvent);
		CloseHandle(this->sleepEvent);
		DeleteCriticalSection(&pauseSync);


		if(this->gfxbuffer)
		{
			delete [] this->gfxbuffer;
			this->gfxbuffer = nullptr;
		}

		this->ReleaseAllResources();

		delete [] this->gfxbuffer;

	}

	void EmulatorGame::ReleaseAllResources(void)
	{
		delete this->p1Controller;
		delete this->keyboard;
		delete this->virtualInput;

		this->DeInitSound();
		
		this->graphicsResourcesReleased = true;
	}
	
	void EmulatorGame::Start(void)
	{
		SetEvent(this->updateEvent);
	}


	void EmulatorGame::Initialize()
	{
		this->p1Controller = new ControllerInput(1);
		this->keyboard = new KeyboardInput();
		this->virtualInput = new VirtualControllerInput();
		
		this->updateCount = 0;
		this->frameSkipped = false;


		if(!this->graphicsResourcesReleased)
		{
			this->InitSound();
		
			systemColorDepth = 32;
			systemRedShift = 19;
			systemBlueShift = 3;
			systemGreenShift = 11;

			utilUpdateSystemColorMaps();

			if(this->restoreState)
			{
				this->restoreState = false;
				RestoreFromApplicationDataAsync();
			}

			RestoreSettings();

			// Start Snes9x Thread
			InitializeCriticalSectionEx(&pauseSync, NULL, NULL);
			this->flipBufferEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_INITIAL_SET, EVENT_ALL_ACCESS);
			this->updateEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
			this->sleepEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
			this->threadAction = ThreadPool::RunAsync(ref new WorkItemHandler([this](IAsyncAction ^action)
			{
				this->UpdateAsync();	
			}), WorkItemPriority::High, WorkItemOptions::None);
		}else
		{
			/*ResetEvent(this->updateEvent);
			SetEvent(this->flipBufferEvent);*/
			ResetEvent(this->flipBufferEvent);
			ResetEvent(this->updateEvent);
		}

		this->graphicsResourcesReleased = false;
	}

	void EmulatorGame::InitSound(void)
	{
		if(soundDriver)
		{
			soundDriver->init(soundSampleRate);
			if(true)//EmulatorSettings::Current->SoundEnabled)
			{
				soundSetVolume(1.0f);
			}else
			{
				soundSetVolume(0.0f);
			}
		}
	}
		
	void EmulatorGame::DeInitSound(void)
	{
		if(soundDriver)
		{
			soundDriver->close();
		}
	}

	VirtualControllerInput *EmulatorGame::GetVirtualController(void) const
	{
		return this->virtualInput;
	}
	
	KeyboardInput *EmulatorGame::GetKeyboardInput(void) const
	{
		return this->keyboard;
	}

	ControllerInput *EmulatorGame::GetControllerInput(void) const
	{
		return this->p1Controller;
	}

	void EmulatorGame::FocusChanged(bool focus)
	{
		this->focus = focus;
	}

	void uSleep(int waitTime) 
	{
		__int64 time1 = 0, time2 = 0, freq = 0;

		QueryPerformanceCounter((LARGE_INTEGER *) &time1);
		QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

		do {
			QueryPerformanceCounter((LARGE_INTEGER *) &time2);
		} while((time2-time1) < waitTime);
	}

	void EmulatorGame::ResizeBuffer(float width, float height)
	{
		this->width = width;
		this->height = height;
		this->virtualInput->UpdateVirtualControllerRectangles();
	}

	task<void> EmulatorGame::StopROMAsync(void)
	{
		return create_task([this]()
		{
			if(IsROMLoaded())
			{
				this->Pause();
				SaveSRAMAsync().wait();
				int oldstate = SavestateSlot;
				SavestateSlot = AUTOSAVESTATE_SLOT;
				SaveStateAsync().wait();
				SavestateSlot = oldstate;
				this->Pause();
				//this->InitSound();
				ROMFile = nullptr;
				ROMFolder = nullptr;
				ROMLoaded = false;
				this->frameSkipped = false;
				this->updateCount = 0;
			}
		});
	}

	void EmulatorGame::Pause(void)
	{

		if(emulating)
		{
			//EnterCriticalSection(&pauseSync);
			emulating = false;
		}

	}

	void EmulatorGame::Unpause(void)
	{
		if(IsROMLoaded() && !emulating)
		{

			emulating = true;
			//LeaveCriticalSection(&pauseSync);
		}
	}

	bool EmulatorGame::IsPaused(void)
	{
		//return Settings.StopEmulation;
		return !emulating;
	}
	
	int EmulatorGame::GetWidth(void)
	{
		return this->width;
	}

	int EmulatorGame::GetHeight(void)
	{
		return this->height;
	}

	bool EmulatorGame::LastFrameSkipped(void)
	{
		return this->frameSkipped;
	}

	void EmulatorGame::UpdateAsync(void)
	{
		WaitForSingleObjectEx(this->updateEvent, INFINITE, false);
		while(!this->stopThread)
		{			
			EnterCriticalSection(&pauseSync);
			emulator.emuMain(emulator.emuCount);
			LeaveCriticalSection(&pauseSync);
		}
		SetEvent(this->flipBufferEvent);

#if _DEBUG
		OutputDebugStringW(L"Thread ended.\n");
#endif
	}

	void EmulatorGame::Update(void)
	{		
		this->keyboard->Update();
		this->p1Controller->Update();
		this->virtualInput->Update();

		/*if(timeMeasured && IsROMLoaded() && (!Settings.StopEmulation || autosaving))
		{		
			Settings.Mute = !SoundEnabled();
			
			if(this->focus)
			{
				this->HandleInput();
			}

			this->FlipBuffers(buffer, rowPitch);
			
		}*/
	}

	void EmulatorGame::FlipBuffers(void *buffer, size_t rowPitch)
	{
		WaitForSingleObjectEx(this->flipBufferEvent, INFINITE, false);

		/*GFX.Screen = (uint16*) (buffer);
		GFX.Pitch = rowPitch;*/

		SetEvent(this->updateEvent);
	}
}
