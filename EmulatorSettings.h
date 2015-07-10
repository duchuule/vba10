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

#pragma region Button positions

		property double PadCenterXP
		{
			double get()
			{
				return GetValueOrDefault<double>(PadCenterXPKey, PadCenterXPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(PadCenterXPKey, value);
			}
		}
		property double PadCenterYP
		{
			double get()
			{
				return GetValueOrDefault<double>(PadCenterYPKey, PadCenterYPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(PadCenterYPKey, value);
			}
		}
		property double ALeftP
		{
			double get()
			{
				return GetValueOrDefault<double>(ALeftPKey, ALeftPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ALeftPKey, value);
			}
		}
		property double ATopP
		{
			double get()
			{
				return GetValueOrDefault<double>(ATopPKey, ATopPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ATopPKey, value);
			}
		}
		property double BLeftP
		{
			double get()
			{
				return GetValueOrDefault<double>(BLeftPKey, BLeftPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(BLeftPKey, value);
			}
		}
		property double BTopP
		{
			double get()
			{
				return GetValueOrDefault<double>(BTopPKey, BTopPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(BTopPKey, value);
			}
		}
		property double StartLeftP
		{
			double get()
			{
				return GetValueOrDefault<double>(StartLeftPKey, StartLeftPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(StartLeftPKey, value);
			}
		}
		property double StartTopP
		{
			double get()
			{
				return GetValueOrDefault<double>(StartTopPKey, StartTopPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(StartTopPKey, value);
			}
		}
		property double SelectRightP
		{
			double get()
			{
				return GetValueOrDefault<double>(SelectRightPKey, SelectRightPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(SelectRightPKey, value);
			}
		}
		property double SelectTopP
		{
			double get()
			{
				return GetValueOrDefault<double>(SelectTopPKey, SelectTopPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(SelectTopPKey, value);
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
		property double LTopP
		{
			double get()
			{
				return GetValueOrDefault<double>(LTopPKey, LTopPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(LTopPKey, value);
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
		property double RTopP
		{
			double get()
			{
				return GetValueOrDefault<double>(RTopPKey, RTopPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(RTopPKey, value);
			}
		}
		property double TurboLeftP
		{
			double get()
			{
				return GetValueOrDefault<double>(TurboLeftPKey, TurboLeftPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(TurboLeftPKey, value);
			}
		}
		property double TurboTopP
		{
			double get()
			{
				return GetValueOrDefault<double>(TurboTopPKey, TurboTopPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(TurboTopPKey, value);
			}
		}
		property double ComboLeftP
		{
			double get()
			{
				return GetValueOrDefault<double>(ComboLeftPKey, ComboLeftPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ComboLeftPKey, value);
			}
		}
		property double ComboTopP
		{
			double get()
			{
				return GetValueOrDefault<double>(ComboTopPKey, ComboTopPDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ComboTopPKey, value);
			}
		}

		property double PadCenterXL
		{
			double get()
			{
				return GetValueOrDefault<double>(PadCenterXLKey, PadCenterXLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(PadCenterXLKey, value);
			}
		}
		property double PadCenterYL
		{
			double get()
			{
				return GetValueOrDefault<double>(PadCenterYLKey, PadCenterYLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(PadCenterYLKey, value);
			}
		}
		property double ALeftL
		{
			double get()
			{
				return GetValueOrDefault<double>(ALeftLKey, ALeftLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ALeftLKey, value);
			}
		}
		property double ATopL
		{
			double get()
			{
				return GetValueOrDefault<double>(ATopLKey, ATopLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ATopLKey, value);
			}
		}
		property double BLeftL
		{
			double get()
			{
				return GetValueOrDefault<double>(BLeftLKey, BLeftLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(BLeftLKey, value);
			}
		}
		property double BTopL
		{
			double get()
			{
				return GetValueOrDefault<double>(BTopLKey, BTopLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(BTopLKey, value);
			}
		}
		property double StartLeftL
		{
			double get()
			{
				return GetValueOrDefault<double>(StartLeftLKey, StartLeftLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(StartLeftLKey, value);
			}
		}
		property double StartTopL
		{
			double get()
			{
				return GetValueOrDefault<double>(StartTopLKey, StartTopLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(StartTopLKey, value);
			}
		}
		property double SelectRightL
		{
			double get()
			{
				return GetValueOrDefault<double>(SelectRightLKey, SelectRightLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(SelectRightLKey, value);
			}
		}
		property double SelectTopL
		{
			double get()
			{
				return GetValueOrDefault<double>(SelectTopLKey, SelectTopLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(SelectTopLKey, value);
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
		property double LTopL
		{
			double get()
			{
				return GetValueOrDefault<double>(LTopLKey, LTopLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(LTopLKey, value);
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
		property double RTopL
		{
			double get()
			{
				return GetValueOrDefault<double>(RTopLKey, RTopLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(RTopLKey, value);
			}
		}
		property double TurboLeftL
		{
			double get()
			{
				return GetValueOrDefault<double>(TurboLeftLKey, TurboLeftLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(TurboLeftLKey, value);
			}
		}
		property double TurboTopL
		{
			double get()
			{
				return GetValueOrDefault<double>(TurboTopLKey, TurboTopLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(TurboTopLKey, value);
			}
		}
		property double ComboLeftL
		{
			double get()
			{
				return GetValueOrDefault<double>(ComboLeftLKey, ComboLeftLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ComboLeftLKey, value);
			}
		}
		property double ComboTopL
		{
			double get()
			{
				return GetValueOrDefault<double>(ComboTopLKey, ComboTopLDefault);
			}
			void set(double value)
			{
				AddOrUpdateValue(ComboTopLKey, value);
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

#pragma region button positions
		Platform::String^ PadCenterXPKey = "PadCenterXPKey";
		Platform::String^ PadCenterYPKey = "PadCenterYPKey";
		Platform::String^ ALeftPKey = "ALeftPKey";
		Platform::String^ ATopPKey = "ATopPKey";
		Platform::String^ BLeftPKey = "BLeftPKey";
		Platform::String^ BTopPKey = "BTopPKey";
		Platform::String^ StartLeftPKey = "StartLeftPKey";
		Platform::String^ StartTopPKey = "StartTopPKey";
		Platform::String^ SelectRightPKey = "SelectRightPKey";
		Platform::String^ SelectTopPKey = "SelectTopPKey";
		Platform::String^ LLeftPKey = "LLeftPKey";
		Platform::String^ LTopPKey = "LTopPKey";
		Platform::String^ RRightPKey = "RRightPKey";
		Platform::String^ RTopPKey = "RTopPKey";
		Platform::String^ TurboLeftPKey = "TurboLeftPKey";
		Platform::String^ TurboTopPKey = "TurboTopPKey";
		Platform::String^ ComboLeftPKey = "ComboLeftPKey";
		Platform::String^ ComboTopPKey = "ComboTopPKey";

		Platform::String^ PadCenterXLKey = "PadCenterXLKey";
		Platform::String^ PadCenterYLKey = "PadCenterYLKey";
		Platform::String^ ALeftLKey = "ALeftLKey";
		Platform::String^ ATopLKey = "ATopLKey";
		Platform::String^ BLeftLKey = "BLeftLKey";
		Platform::String^ BTopLKey = "BTopLKey";
		Platform::String^ StartLeftLKey = "StartLeftLKey";
		Platform::String^ StartTopLKey = "StartTopLKey";
		Platform::String^ SelectRightLKey = "SelectRightLKey";
		Platform::String^ SelectTopLKey = "SelectTopLKey";
		Platform::String^ LLeftLKey = "LLeftLKey";
		Platform::String^ LTopLKey = "LTopLKey";
		Platform::String^ RRightLKey = "RRightLKey";
		Platform::String^ RTopLKey = "RTopLKey";
		Platform::String^ TurboLeftLKey = "TurboLeftLKey";
		Platform::String^ TurboTopLKey = "TurboTopLKey";
		Platform::String^ ComboLeftLKey = "ComboLeftLKey";
		Platform::String^ ComboTopLKey = "ComboTopLKey";
#pragma endregion

		// The default value of our settings
		const bool LinearFilterDefault = true;
		const int ControllerScaleDefault = 100;
		const int ButtonScaleDefault = 100;
		const int DPadStyleDefault = 0;

#pragma region button positions
		const double PadCenterXPDefault = 0.25f;
		const double PadCenterYPDefault = 0.7f;
		const double ALeftPDefault = 0.75f;
		const double ATopPDefault = 0.6f;
		const double BLeftPDefault = 0.55f;
		const double BTopPDefault = 0.72f;
		const double StartLeftPDefault = 0.53f;
		const double StartTopPDefault = 0.93f;
		const double SelectRightPDefault = 0.47f;
		const double SelectTopPDefault = 0.93f;
		const double LLeftPDefault = 0.0f;
		const double LTopPDefault = 0.87f;
		const double RRightPDefault = 1.0f;
		const double RTopPDefault = 0.87f;
		const double TurboLeftPDefault = 0.6f;
		const double TurboTopPDefault = 0.63f;
		const double ComboLeftPDefault = 0.85f;
		const double ComboTopPDefault = 0.77f;

		const double PadCenterXLDefault = 0.15f;
		const double PadCenterYLDefault = 0.75f;
		const double ALeftLDefault = 0.82f;
		const double ATopLDefault = 0.45f;
		const double BLeftLDefault = 0.71f;
		const double BTopLDefault = 0.69f;
		const double StartLeftLDefault = 0.53f;
		const double StartTopLDefault = 0.90f;
		const double SelectRightLDefault = 0.47f;
		const double SelectTopLDefault = 0.90f;
		const double LLeftLDefault = 0.0f;
		const double LTopLDefault = 0.3f;
		const double RRightLDefault = 1.0f;
		const double RTopLDefault = 0.3f;
		const double TurboLeftLDefault = 0.74f;
		const double TurboTopLDefault = 0.55f;
		const double ComboLeftLDefault = 0.89f;
		const double ComboTopLDefault = 0.72f;
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

	void SetFrameSkip(int skip);
	int GetFrameSkip(void);
	void SetTurboFrameSkip(int skip);
	int GetTurboFrameSkip(void);
	void SetPowerFrameSkip(int skip);
	int GetPowerFrameSkip(void);
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