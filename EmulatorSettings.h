#pragma once

#include "Emulator.h"

using namespace Windows::System;
using namespace Windows::Storage;

#define MONITOR_30HZ	0
#define MONITOR_60HZ	1
#define MONITOR_120HZ	2

namespace VBA10
{		
	

	ref  class EmulatorSettings sealed
	{
	public:
		EmulatorSettings();

		static property EmulatorSettings ^Current
		{
			EmulatorSettings ^get()
			{
				if (!instance)
				{
					instance = ref new EmulatorSettings();
				}
				return instance;
			}
		}

		
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

		property int ControllerScale
		{
			int get()
			{
				return GetValueOrDefault<int>(ControllerScaleKey, ControllerScaleDefault);
			}
			void set(int value)
			{
				AddOrUpdateValue(ControllerScaleKey, value);
				EmulatorGame::GetInstance()->GetVirtualController()->UpdateVirtualControllerRectangles();
			}
		}


		property int ButtonScale
		{
			int get()
			{
				return GetValueOrDefault<int>(ButtonScaleKey, ButtonScaleDefault);
			}
			void set(int value)
			{
				AddOrUpdateValue(ButtonScaleKey, value);
				EmulatorGame::GetInstance()->GetVirtualController()->UpdateVirtualControllerRectangles();
			}
		}

		property int DPadStyle
		{
			int get()
			{
				return GetValueOrDefault<int>(DPadStyleKey, DPadStyleDefault);
			}
			void set(int value)
			{
				AddOrUpdateValue(DPadStyleKey, value);
				EmulatorGame::GetInstance()->GetVirtualController()->UpdateVirtualControllerRectangles();
			}
		}

		property int TurboFrameSkip
		{
			int get()
			{
				return GetValueOrDefault<int>(TurboFrameSkipKey, TurboFrameSkipDefault);
			}
			void set(int value)
			{
				AddOrUpdateValue(TurboFrameSkipKey, value);
			}
		}

		property bool EnableTurbo
		{
			bool get()
			{
				return GetValueOrDefault<bool>(EnableTurboKey, EnableTurboDefault);
			}
			void set(bool value)
			{
				AddOrUpdateValue(EnableTurboKey, value);
			}
		}

		property bool FullScreen
		{
			bool get()
			{
				return GetValueOrDefault<bool>(FullScreenKey, FullScreenDefault);
			}
			void set(bool value)
			{
				AddOrUpdateValue(FullScreenKey, value);
			}
		}

		//hold setting about previous full screen setting to walk aroudn a bug in ApplicationViewWindowingMode::Auto
		property bool PreviousFullScreen
		{
			bool get()
			{
				return GetValueOrDefault<bool>(PreviousFullScreenKey, PreviousFullScreenDefault);
			}
			void set(bool value)
			{
				AddOrUpdateValue(PreviousFullScreenKey, value);
			}
		}


#pragma region Button positions

		property double PadLeftP
		{
			double get()
			{
				return GetValueOrDefault<double>(PadLeftPKey, PadLeftPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(PadLeftPKey, value);
			}
		}
		property double PadBottomP
		{
			double get()
			{
				return GetValueOrDefault<double>(PadBottomPKey, PadBottomPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(PadBottomPKey, value);
			}
		}
		property double ACenterXP
		{
			double get()
			{
				return GetValueOrDefault<double>(ACenterXPKey, ACenterXPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ACenterXPKey, value);
			}
		}
		property double ACenterYP
		{
			double get()
			{
				return GetValueOrDefault<double>(ACenterYPKey, ACenterYPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ACenterYPKey, value);
			}
		}
		property double BCenterXP
		{
			double get()
			{
				return GetValueOrDefault<double>(BCenterXPKey, BCenterXPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(BCenterXPKey, value);
			}
		}
		property double BCenterYP
		{
			double get()
			{
				return GetValueOrDefault<double>(BCenterYPKey, BCenterYPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(BCenterYPKey, value);
			}
		}
		property double startCenterXP
		{
			double get()
			{
				return GetValueOrDefault<double>(startCenterXPKey, startCenterXPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(startCenterXPKey, value);
			}
		}
		property double startBottomP
		{
			double get()
			{
				return GetValueOrDefault<double>(startBottomPKey, startBottomPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(startBottomPKey, value);
			}
		}
		property double selectCenterXP
		{
			double get()
			{
				return GetValueOrDefault<double>(selectCenterXPKey, selectCenterXPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(selectCenterXPKey, value);
			}
		}
		property double selectBottomP
		{
			double get()
			{
				return GetValueOrDefault<double>(selectBottomPKey, selectBottomPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(selectBottomPKey, value);
			}
		}
		property double LLeftP
		{
			double get()
			{
				return GetValueOrDefault<double>(LLeftPKey, LLeftPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(LLeftPKey, value);
			}
		}
		property double LCenterYP
		{
			double get()
			{
				return GetValueOrDefault<double>(LCenterYPKey, LCenterYPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(LCenterYPKey, value);
			}
		}
		property double RRightP
		{
			double get()
			{
				return GetValueOrDefault<double>(RRightPKey, RRightPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(RRightPKey, value);
			}
		}
		property double RCenterYP
		{
			double get()
			{
				return GetValueOrDefault<double>(RCenterYPKey, RCenterYPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(RCenterYPKey, value);
			}
		}
		property double TurboCenterXP
		{
			double get()
			{
				return GetValueOrDefault<double>(TurboCenterXPKey, TurboCenterXPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(TurboCenterXPKey, value);
			}
		}
		property double TurboCenterYP
		{
			double get()
			{
				return GetValueOrDefault<double>(TurboCenterYPKey, TurboCenterYPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(TurboCenterYPKey, value);
			}
		}
		property double ComboCenterXP
		{
			double get()
			{
				return GetValueOrDefault<double>(ComboCenterXPKey, ComboCenterXPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ComboCenterXPKey, value);
			}
		}
		property double ComboCenterYP
		{
			double get()
			{
				return GetValueOrDefault<double>(ComboCenterYPKey, ComboCenterYPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ComboCenterYPKey, value);
			}
		}

		property double PadLeftL
		{
			double get()
			{
				return GetValueOrDefault<double>(PadLeftLKey, PadLeftLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(PadLeftLKey, value);
			}
		}
		property double PadBottomL
		{
			double get()
			{
				return GetValueOrDefault<double>(PadBottomLKey, PadBottomLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(PadBottomLKey, value);
			}
		}
		property double ACenterXL
		{
			double get()
			{
				return GetValueOrDefault<double>(ACenterXLKey, ACenterXLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ACenterXLKey, value);
			}
		}
		property double ACenterYL
		{
			double get()
			{
				return GetValueOrDefault<double>(ACenterYLKey, ACenterYLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ACenterYLKey, value);
			}
		}
		property double BCenterXL
		{
			double get()
			{
				return GetValueOrDefault<double>(BCenterXLKey, BCenterXLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(BCenterXLKey, value);
			}
		}
		property double BCenterYL
		{
			double get()
			{
				return GetValueOrDefault<double>(BCenterYLKey, BCenterYLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(BCenterYLKey, value);
			}
		}
		property double startCenterXL
		{
			double get()
			{
				return GetValueOrDefault<double>(startCenterXLKey, startCenterXLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(startCenterXLKey, value);
			}
		}
		property double startBottomL
		{
			double get()
			{
				return GetValueOrDefault<double>(startBottomLKey, startBottomLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(startBottomLKey, value);
			}
		}
		property double selectCenterXL
		{
			double get()
			{
				return GetValueOrDefault<double>(selectCenterXLKey, selectCenterXLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(selectCenterXLKey, value);
			}
		}
		property double selectBottomL
		{
			double get()
			{
				return GetValueOrDefault<double>(selectBottomLKey, selectBottomLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(selectBottomLKey, value);
			}
		}
		property double LLeftL
		{
			double get()
			{
				return GetValueOrDefault<double>(LLeftLKey, LLeftLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(LLeftLKey, value);
			}
		}
		property double LCenterYL
		{
			double get()
			{
				return GetValueOrDefault<double>(LCenterYLKey, LCenterYLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(LCenterYLKey, value);
			}
		}
		property double RRightL
		{
			double get()
			{
				return GetValueOrDefault<double>(RRightLKey, RRightLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(RRightLKey, value);
			}
		}
		property double RCenterYL
		{
			double get()
			{
				return GetValueOrDefault<double>(RCenterYLKey, RCenterYLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(RCenterYLKey, value);
			}
		}
		property double TurboCenterXL
		{
			double get()
			{
				return GetValueOrDefault<double>(TurboCenterXLKey, TurboCenterXLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(TurboCenterXLKey, value);
			}
		}
		property double TurboCenterYL
		{
			double get()
			{
				return GetValueOrDefault<double>(TurboCenterYLKey, TurboCenterYLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(TurboCenterYLKey, value);
			}
		}
		property double ComboCenterXL
		{
			double get()
			{
				return GetValueOrDefault<double>(ComboCenterXLKey, ComboCenterXLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ComboCenterXLKey, value);
			}
		}
		property double ComboCenterYL
		{
			double get()
			{
				return GetValueOrDefault<double>(ComboCenterYLKey, ComboCenterYLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ComboCenterYLKey, value);
			}
		}

#pragma endregion

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
		

		static EmulatorSettings ^instance;

		// Our isolated storage settings
		ApplicationDataContainer^ localSettings;
		

		// The isolated storage key names of our settings
		Platform::String^ LinearFilterKey = "LinearFilterKey";
		Platform::String^ ControllerScaleKey = "ControllerScaleKey";
		Platform::String^ ButtonScaleKey = "ButtonScaleKey";
		Platform::String^ DPadStyleKey = "DPadStyleKey";
		Platform::String^ TurboFrameSkipKey = "TurboFrameSkipKey";
		Platform::String^ EnableTurboKey = "EnableTurboKey";
		Platform::String^ FullScreenKey = "FullScreenKey";
		Platform::String^ PreviousFullScreenKey = "PreviousFullScreenKey";

#pragma region button positions
		Platform::String^ PadLeftPKey = "PadLeftPKey";
		Platform::String^ PadBottomPKey = "PadBottomPKey";
		Platform::String^ ACenterXPKey = "ACenterXPKey";
		Platform::String^ ACenterYPKey = "ACenterYPKey";
		Platform::String^ BCenterXPKey = "BCenterXPKey";
		Platform::String^ BCenterYPKey = "BCenterYPKey";
		Platform::String^ startCenterXPKey = "startCenterXPKey";
		Platform::String^ startBottomPKey = "startBottomPKey";
		Platform::String^ selectCenterXPKey = "selectCenterXPKey";
		Platform::String^ selectBottomPKey = "selectBottomPKey";
		Platform::String^ LLeftPKey = "LLeftPKey";
		Platform::String^ LCenterYPKey = "LCenterYPKey";
		Platform::String^ RRightPKey = "RRightPKey";
		Platform::String^ RCenterYPKey = "RCenterYPKey";
		Platform::String^ TurboCenterXPKey = "TurboCenterXPKey";
		Platform::String^ TurboCenterYPKey = "TurboCenterYPKey";
		Platform::String^ ComboCenterXPKey = "ComboCenterXPKey";
		Platform::String^ ComboCenterYPKey = "ComboCenterYPKey";

		Platform::String^ PadLeftLKey = "PadLeftLKey";
		Platform::String^ PadBottomLKey = "PadBottomLKey";
		Platform::String^ ACenterXLKey = "ACenterXLKey";
		Platform::String^ ACenterYLKey = "ACenterYLKey";
		Platform::String^ BCenterXLKey = "BCenterXLKey";
		Platform::String^ BCenterYLKey = "BCenterYLKey";
		Platform::String^ startCenterXLKey = "startCenterXLKey";
		Platform::String^ startBottomLKey = "startBottomLKey";
		Platform::String^ selectCenterXLKey = "selectCenterXLKey";
		Platform::String^ selectBottomLKey = "selectBottomLKey";
		Platform::String^ LLeftLKey = "LLeftLKey";
		Platform::String^ LCenterYLKey = "LCenterYLKey";
		Platform::String^ RRightLKey = "RRightLKey";
		Platform::String^ RCenterYLKey = "RCenterYLKey";
		Platform::String^ TurboCenterXLKey = "TurboCenterXLKey";
		Platform::String^ TurboCenterYLKey = "TurboCenterYLKey";
		Platform::String^ ComboCenterXLKey = "ComboCenterXLKey";
		Platform::String^ ComboCenterYLKey = "ComboCenterYLKey";
#pragma endregion

		// The default value of our settings
		const bool LinearFilterDefault = true;
		const int ControllerScaleDefault = 100;
		const int ButtonScaleDefault = 100;
		const int DPadStyleDefault = 0;
		const int TurboFrameSkipDefault = 2;
		const bool EnableTurboDefault = false;
		const bool FullScreenDefault = false;
		const bool PreviousFullScreenDefault = false;

#pragma region button positions (in cm based on 6x10cm phone)
		const double PadLeftPDefault = 0.1f; //from left
		const double PadBottomPDefault = 0.9f; //from bottom
		const double ACenterXPDefault = 0.6f;  //from right
		const double ACenterYPDefault = 2.8f;  //from bottom
		const double BCenterXPDefault = 1.7f; //from right
		const double BCenterYPDefault = 1.8f; //from bottom
		const double startCenterXPDefault = 0.85f; //from center
		const double startBottomPDefault = 0.1f;  //from bottom
		const double selectCenterXPDefault = -0.85f;  //from center
		const double selectBottomPDefault = 0.1f;  //from bottom
		const double LLeftPDefault = 0.0f;  //from left
		const double LCenterYPDefault = 4.0f;  ///from bottom
		const double RRightPDefault = 0.0f;  //from right
		const double RCenterYPDefault = 4.0f;  //from bottom
		const double TurboCenterXPDefault = 1.8f;  //from right
		const double TurboCenterYPDefault = 2.8f; //from bottom
		const double ComboCenterXPDefault = 0.6f;  //from right
		const double ComboCenterYPDefault = 1.8f; //from bottom

		const double PadLeftLDefault = 0.3f; //from left
		const double PadBottomLDefault = 0.3f; //from bottom
		const double ACenterXLDefault = 0.8f; //from right
		const double ACenterYLDefault = 2.1f; //from bottom
		const double BCenterXLDefault = 2.0f; //from right
		const double BCenterYLDefault = 1.0f; //from bottom
		const double startCenterXLDefault = 1.0f;
		const double startBottomLDefault = 0.3f;
		const double selectCenterXLDefault = -1.0f;
		const double selectBottomLDefault = 0.3f;
		const double LLeftLDefault = 0.0f;
		const double LCenterYLDefault = 3.8f;
		const double RRightLDefault = 0.0f;
		const double RCenterYLDefault = 3.8f;
		const double TurboCenterXLDefault = 2.0f;
		const double TurboCenterYLDefault = 2.1f;
		const double ComboCenterXLDefault = 0.8f;
		const double ComboCenterYLDefault = 1.1f;
#pragma endregion

	};


	public enum class AspectRatioMode : int
	{
		Original,
		Stretch,
		One,
		FourToThree,
		FiveToFour,
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

	void SetControllerOpacity(int opacity);
	int GetControllerOpacity(void);


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
	float GetDeadzone(void);
	void SetDeadzone(float value);
	int GetMonitorType(void);
	void SetMonitorType(int type);

	bool IsSaveConfirmationDisabled(void);
	void DisableSaveConfirmation(bool disable);

	bool IsLoadConfirmationDisabled(void);
	void DisableLoadConfirmation(bool disable);



	
}