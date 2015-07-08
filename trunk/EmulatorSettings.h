#pragma once

#include "Emulator.h"

using namespace Windows::System;
using namespace Windows::Storage;

#define MONITOR_30HZ	0
#define MONITOR_60HZ	1
#define MONITOR_120HZ	2

namespace VBA10
{		
	public enum class AspectRatioMode : int
	{
		Original,
		Stretch,
		One,
		FourToThree,
		FiveToFour,
	};

	ref class EmulatorSettings sealed
	{
	public:
		EmulatorSettings();
		
		property bool LinearFilterEnabled
		{
			bool get()
			{
				return GetValueOrDefault<bool> (LinearFilterKey, LinearFilterDefault);
			}
			void set (bool value)
			{
				AddOrUpdateValue(LinearFilterKey, value);
			}
		}

	private:
		void AddOrUpdateValue(Platform::String^ key, Platform::Object^ value);
		bool GetValueOrDefault(Platform::String^ key, bool defaultValue);
		
		template<typename T>
		T GetValueOrDefault(Platform::String^ key, T defaultValue)
		{

			if (localSettings->Values->HasKey(key))
			{
				return (T)localSettings->Values->Lookup(key);
			}
			else
			{
				return defaultValue;
			}
		}
		
	

		// Our isolated storage settings
		ApplicationDataContainer^ localSettings;

		// The isolated storage key names of our settings
		Platform::String^ LinearFilterKey = "LinearFilter1";

		// The default value of our settings
		const bool LinearFilterDefault = true;

		

	};


	void StoreSettings(void);
	void RestoreSettings(void);

	VirtualKey GetLeftKeyBinding(void);
	void SetLeftKeyBinding(VirtualKey key);
	VirtualKey GetRightKeyBinding(void);
	void SetRightKeyBinding(VirtualKey key);
	VirtualKey GetUpKeyBinding(void);
	void SetUpKeyBinding(VirtualKey key);
	VirtualKey GetDownKeyBinding(void);
	void SetDownKeyBinding(VirtualKey key);
	VirtualKey GetStartKeyBinding(void);
	void SetStartKeyBinding(VirtualKey key);
	VirtualKey GetSelectKeyBinding(void);
	void SetSelectKeyBinding(VirtualKey key);
	VirtualKey GetAKeyBinding(void);
	void SetAKeyBinding(VirtualKey key);
	VirtualKey GetBKeyBinding(void);
	void SetBKeyBinding(VirtualKey key);
	VirtualKey GetTurboKeyBinding(void);
	void SetTurboKeyBinding(VirtualKey key);
	VirtualKey GetLKeyBinding(void);
	void SetLKeyBinding(VirtualKey key);
	VirtualKey GetRKeyBinding(void);
	void SetRKeyBinding(VirtualKey key);

	void SetFrameSkip(int skip);
	int GetFrameSkip(void);
	void SetTurboFrameSkip(int skip);
	int GetTurboFrameSkip(void);
	void SetPowerFrameSkip(int skip);
	int GetPowerFrameSkip(void);
	void SetControllerOpacity(int opacity);
	int GetControllerOpacity(void);
	int GetControllerScaling(void);
	void SetControllerScaling(int scaling);

	void SetAspectRatio(AspectRatioMode aspect);
	AspectRatioMode GetAspectRatio(void);
	void SetImageScale(int scale);
	int GetImageScale(void);
	void EnableAutosaving(bool enable);
	bool AutosavingEnabled(void);
	bool IsTouchControllerOnTop(void);
	void SetTouchControllerOnTop(bool onTop);
	/*bool LowDisplayRefreshModeActivated(void);
	void EnableLowDisplayRefreshMode(bool enable);*/
	bool TouchControlsEnabled(void);
	void EnableTouchControls(bool enable);
	bool SoundEnabled(void);
	void EnableSound(bool enable);
	bool ShowingFPS(void);
	void ShowFPS(bool show);
	bool SynchronizeAudio(void);
	void SetSynchronizeAudio(bool sync);
	int GetDPadStyle(void);
	void SetDPadStyle(int dpad);
	float GetDeadzone(void);
	void SetDeadzone(float value);
	int GetMonitorType(void);
	void SetMonitorType(int type);

	bool IsSaveConfirmationDisabled(void);
	void DisableSaveConfirmation(bool disable);

	bool IsLoadConfirmationDisabled(void);
	void DisableLoadConfirmation(bool disable);

	void EnableLinearFilter(bool enable);
	bool IsLinearFilterEnabled(void);

	
}