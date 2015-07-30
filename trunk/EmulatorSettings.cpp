#include "EmulatorSettings.h"

using namespace Windows::Foundation;

#define DEFAULT_SHOW_FPS_VALUE		true
#define DEFAULT_SOUND_ENABLED_VALUE	true
#define DEFAULT_TOUCH_ENABLED_VALUE	true
#define DEFAULT_TOUCH_ON_TOP		false
#define DEFAULT_CONTROL_ASSIGNED_TO_P2 false
#define DEFAULT_TOUCH_BUTTONS_INTERSECTING	false
#define DEFAULT_AUTOSAVING_ENABLED true
#define DEFAULT_GRAY_BUTTONS	false
#define DEFAULT_CONTROLLER_SCALE	100
#define DEFAULT_CONTROLLER_OPACITY	50
#define DEFAULT_ASPECT			AspectRatioMode::Original
#define DEFAULT_IMAGE_SCALE		100
#define DEFAULT_SYNC_AUDIO		false
#define DEFAULT_DPAD_STYLE		1
#define DEFAULT_DEADZONE_VALUE	10.0f
#define DEFAULT_MONITOR_TYPE	MONITOR_60HZ
#define DEFAULT_SAVE_CONFIRM	false
#define DEFAULT_LOAD_CONFIRM	false
#define DEFAULT_LINEAR_FILTER	true

#define DEFAULT_LEFT_KEY	65
#define DEFAULT_RIGHT_KEY	68
#define DEFAULT_UP_KEY		87
#define DEFAULT_DOWN_KEY	83
#define DEFAULT_START_KEY	32
#define DEFAULT_SELECT_KEY	16
#define DEFAULT_A_KEY		75
#define DEFAULT_B_KEY		74
#define DEFAULT_X_KEY		38
#define DEFAULT_Y_KEY		37
#define DEFAULT_L_KEY		81
#define DEFAULT_R_KEY		69
#define DEFAULT_TURBO_KEY	84

extern bool soundInit(void);
extern bool synchronize;

namespace VBA10
{
	EmulatorSettings ^EmulatorSettings::instance;

	EmulatorSettings::EmulatorSettings()
	{

		localSettings = ApplicationData::Current->LocalSettings;
	}

	void EmulatorSettings::AddOrUpdateValue( Platform::String^ key, Platform::Object^ value)
	{
		localSettings->Values->Insert(key, value);

	}

	



	


	//bool timeMeasured = false;
	bool showFPS = false;

	bool soundEnabled = true;
	bool touchEnabled = true;
	//bool lowFrequencyMode = false;
	bool touchControllerOnTop = true;
	bool autosavingEnabled = true;


	int controllerOpacity = 50;
	AspectRatioMode aspect = AspectRatioMode::Original;
	int imageScale = 100;
	float deadzone = 10.0f;
	int monitorType = 1;
	bool saveConfirmDisabled = false;
	bool loadConfirmDisabled = false;
	bool linearFilterEnabled = true;

	VirtualKey leftKey;
	VirtualKey upKey;
	VirtualKey rightKey;
	VirtualKey downKey;
	VirtualKey bKey;
	VirtualKey aKey;
	VirtualKey startKey;
	VirtualKey selectKey;
	VirtualKey lKey;
	VirtualKey rKey;
	VirtualKey turboKey;
		
	void setLeftKeyBinding(VirtualKey key);
	void setRightKeyBinding(VirtualKey key);
	void setUpKeyBinding(VirtualKey key);
	void setDownKeyBinding(VirtualKey key);
	void setStartKeyBinding(VirtualKey key);
	void setSelectKeyBinding(VirtualKey key);
	void setAKeyBinding(VirtualKey key);
	void setBKeyBinding(VirtualKey key);
	void setLKeyBinding(VirtualKey key);
	void setRKeyBinding(VirtualKey key);
	void setTurboKeyBinding(VirtualKey key);
	void setSynchronizeAudio(bool sync);	
	void setMonitorType(int type);

	//void enableLowDisplayRefreshMode(bool enable);
	void enableTouchControls(bool enable);
	void enableSound(bool enable);
	void setTouchControllerOnTop(bool onTop);
	void enableShowFPS(bool show);
	void enableAutosaving(bool enable);
	void setControllerOpacity(int opacity);
	void setAspectRatio(AspectRatioMode aspect);
	void setImageScale(int scale);
	void disableSaveConfirmation(bool disable);
	void disableLoadConfirmation(bool disable);
	void enableLinearFilter(bool enable);
	
	
	void setMonitorType(int type)
	{
		monitorType = type;
	}

	int GetMonitorType(void)
	{
		return monitorType;
	}

	void SetMonitorType(int type)
	{
		setMonitorType(type);
		StoreSettings();
	}
	

	
	void setSynchronizeAudio(bool sync)
	{
		synchronize = sync;
	}

	bool SynchronizeAudio(void)
	{
		return synchronize;
	}

	void SetSynchronizeAudio(bool sync)
	{
		setSynchronizeAudio(sync);
		StoreSettings();
	}

	void setTurboKeyBinding(VirtualKey key)
	{
		turboKey = key;
	}
	
	VirtualKey GetTurboKeyBinding(void)
	{
		return turboKey;
	}

	void SetTurboKeyBinding(VirtualKey key)
	{
		setTurboKeyBinding(key);
		StoreSettings();
	}

	VirtualKey GetLeftKeyBinding(void)
	{
		return leftKey;
	}

	void setLeftKeyBinding(VirtualKey key)
	{
		leftKey = key;
	}

	void SetLeftKeyBinding(VirtualKey key)
	{
		setLeftKeyBinding(key);
		StoreSettings();
	}

	VirtualKey GetRightKeyBinding(void)
	{
		return rightKey;
	}

	void setRightKeyBinding(VirtualKey key)
	{
		rightKey = key;
	}

	void SetRightKeyBinding(VirtualKey key)
	{
		setRightKeyBinding(key);
		StoreSettings();
	}

	VirtualKey GetUpKeyBinding(void)
	{
		return upKey;
	}

	void setUpKeyBinding(VirtualKey key)
	{
		upKey = key;
	}

	void SetUpKeyBinding(VirtualKey key)
	{
		setUpKeyBinding(key);
		StoreSettings();
	}

	VirtualKey GetDownKeyBinding(void)
	{
		return downKey;
	}

	void setDownKeyBinding(VirtualKey key)
	{
		downKey = key;
	}

	void SetDownKeyBinding(VirtualKey key)
	{
		setDownKeyBinding(key);
		StoreSettings();
	}

	VirtualKey GetStartKeyBinding(void)
	{
		return startKey;
	}

	void setStartKeyBinding(VirtualKey key)
	{
		startKey = key;
	}

	void SetStartKeyBinding(VirtualKey key)
	{
		setStartKeyBinding(key);
		StoreSettings();
	}

	VirtualKey GetSelectKeyBinding(void)
	{
		return selectKey;
	}

	void setSelectKeyBinding(VirtualKey key)
	{
		selectKey = key;
	}

	void SetSelectKeyBinding(VirtualKey key)
	{
		setSelectKeyBinding(key);
		StoreSettings();
	}

	VirtualKey GetAKeyBinding(void)
	{
		return aKey;
	}

	void setAKeyBinding(VirtualKey key)
	{
		aKey = key;
	}

	void SetAKeyBinding(VirtualKey key)
	{
		setAKeyBinding(key);
		StoreSettings();
	}

	VirtualKey GetBKeyBinding(void)
	{
		return bKey;
	}

	void setBKeyBinding(VirtualKey key)
	{
		bKey = key;
	}

	void SetBKeyBinding(VirtualKey key)
	{
		setBKeyBinding(key);
		StoreSettings();
	}

	VirtualKey GetLKeyBinding(void)
	{
		return lKey;
	}

	void setLKeyBinding(VirtualKey key)
	{
		lKey = key;
	}

	void SetLKeyBinding(VirtualKey key)
	{
		setLKeyBinding(key);
		StoreSettings();
	}

	VirtualKey GetRKeyBinding(void)
	{
		return rKey;
	}

	void setRKeyBinding(VirtualKey key)
	{
		rKey = key;
	}

	void SetRKeyBinding(VirtualKey key)
	{
		setRKeyBinding(key);
		StoreSettings();
	}

	void SetTouchControllerOnTop(bool onTop)
	{
		setTouchControllerOnTop(onTop);
		StoreSettings();
	}

	void setTouchControllerOnTop(bool onTop)
	{
		touchControllerOnTop = onTop;
		EmulatorGame *emulator = EmulatorGame::GetInstance();
		emulator->GetVirtualController()->UpdateVirtualControllerRectangles();
	}
	
	bool IsTouchControllerOnTop(void)
	{
		return touchControllerOnTop;
	}
	
	/*bool LowDisplayRefreshModeActivated(void)
	{
		return lowFrequencyMode;
	}

	void EnableLowDisplayRefreshMode(bool enable)
	{
		enableLowDisplayRefreshMode(enable);
		StoreSettings();
	}

	void enableLowDisplayRefreshMode(bool enable)
	{
		lowFrequencyMode = enable;
		timeMeasured = true;
	}*/

	bool TouchControlsEnabled(void)
	{
		return touchEnabled;
	}

	void EnableTouchControls(bool enable)
	{
		enableTouchControls(enable);
		StoreSettings();
	}

	void enableTouchControls(bool enable)
	{
		touchEnabled = enable;
	}

	void ShowFPS(bool show)
	{
		enableShowFPS(show);
		StoreSettings();
	}

	void enableShowFPS(bool show)
	{
		showFPS = show;
	}

	bool ShowingFPS(void)
	{
		return showFPS;
	}
	
	bool SoundEnabled(void)
	{
		return soundEnabled;
	}

	void EnableSound(bool enable)
	{
		enableSound(enable);
		StoreSettings();
	}

	void enableSound(bool enable)
	{
		soundEnabled = enable;
		soundInit();
		//Settings.Mute = !enable;
	}
	
	void enableAutosaving(bool enable)
	{
		autosavingEnabled = enable;
	}
	
	void EnableAutosaving(bool enable)
	{
		enableAutosaving(enable);
		StoreSettings();
	}

	bool AutosavingEnabled(void)
	{
		return autosavingEnabled;
	}



	void setAspectRatio(AspectRatioMode aspect)
	{
		VBA10::aspect = aspect;
	}

	void SetAspectRatio(AspectRatioMode aspect)
	{
		setAspectRatio(aspect);
		StoreSettings();
	}

	AspectRatioMode GetAspectRatio(void)
	{
		return aspect;
	}

	void setImageScale(int scale)
	{
		imageScale = scale;
	}

	void SetImageScale(int scale)
	{
		setImageScale(scale);
		StoreSettings();
	}

	int GetImageScale(void)
	{
		return imageScale;
	}



	void setControllerOpacity(int opacity)
	{
		controllerOpacity = opacity;
		StoreSettings();
	}

	void SetControllerOpacity(int opacity)
	{
		setControllerOpacity(opacity);
		StoreSettings();
	}

	int GetControllerOpacity(void)
	{
		return controllerOpacity;
	}
	


	void setDeadzone(float value)
	{
		deadzone = value;
	}

	void SetDeadzone(float value)
	{
		setDeadzone(value);
		StoreSettings();
	}

	float GetDeadzone(void)
	{
		return deadzone;
	}

	void disableSaveConfirmation(bool disable)
	{
		saveConfirmDisabled = disable;
	}

	void DisableSaveConfirmation(bool disable)
	{
		disableSaveConfirmation(disable);
		StoreSettings();
	}

	bool IsSaveConfirmationDisabled(void)
	{
		return saveConfirmDisabled;
	}

	

	void disableLoadConfirmation(bool disable)
	{
		loadConfirmDisabled = disable;
	}	

	void DisableLoadConfirmation(bool disable)
	{
		disableLoadConfirmation(disable);
		StoreSettings();
	}

	bool IsLoadConfirmationDisabled(void)
	{
		return loadConfirmDisabled;
	}
	


	

	

	void StoreSettings(void)
	{
		ApplicationDataContainer ^localSettings = ApplicationData::Current->LocalSettings;
		auto values = localSettings->Values;
		values->Insert("ShowFPS", dynamic_cast<PropertyValue^>(PropertyValue::CreateBoolean(showFPS)));	
		values->Insert("SoundEnabled", dynamic_cast<PropertyValue^>(PropertyValue::CreateBoolean(soundEnabled)));
		values->Insert("TouchEnabled", dynamic_cast<PropertyValue^>(PropertyValue::CreateBoolean(touchEnabled)));
		//values->Insert("LowRefreshMode", dynamic_cast<PropertyValue^>(PropertyValue::CreateBoolean(lowFrequencyMode)));
		values->Insert("LeftKeyMapping", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32((int)leftKey)));
		values->Insert("RightKeyMapping", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32((int)rightKey)));
		values->Insert("DownKeyMapping", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32((int)downKey)));
		values->Insert("UpKeyMapping", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32((int)upKey)));
		values->Insert("LKeyMapping", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32((int)lKey)));
		values->Insert("RKeyMapping", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32((int)rKey)));
		values->Insert("StartKeyMapping", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32((int)startKey)));
		values->Insert("SelectKeyMapping", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32((int)selectKey)));
		values->Insert("BKeyMapping", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32((int)bKey)));
		values->Insert("AKeyMapping", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32((int)aKey)));
		values->Insert("TurboKeyMapping", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32((int)turboKey)));
		values->Insert("TouchControllerOnTop", dynamic_cast<PropertyValue^>(PropertyValue::CreateBoolean(touchControllerOnTop)));
		values->Insert("EnableAutosaving", dynamic_cast<PropertyValue^>(PropertyValue::CreateBoolean(autosavingEnabled)));
		values->Insert("AspectRatio", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32((int) aspect)));
		values->Insert("ImageScale", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32(imageScale)));
		values->Insert("ControllerOpacity", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32(controllerOpacity)));
		values->Insert("SynchronizeAudio", dynamic_cast<PropertyValue^>(PropertyValue::CreateBoolean(synchronize)));
		values->Insert("Deadzone", dynamic_cast<PropertyValue^>(PropertyValue::CreateSingle(deadzone)));
		values->Insert("MonitorType", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32(monitorType)));
		values->Insert("LoadConfirmation", dynamic_cast<PropertyValue^>(PropertyValue::CreateBoolean(loadConfirmDisabled)));
		values->Insert("SaveConfirmation", dynamic_cast<PropertyValue^>(PropertyValue::CreateBoolean(saveConfirmDisabled)));
	}


	void RestoreSettings(void)
	{
		ApplicationDataContainer ^localSettings = ApplicationData::Current->LocalSettings;
		auto values = localSettings->Values;
		auto loadConfirmEntry = safe_cast<IPropertyValue^>(values->Lookup("LoadConfirmation"));
		auto saveConfirmEntry = safe_cast<IPropertyValue^>(values->Lookup("SaveConfirmation"));
		auto showFpsEntry = safe_cast<IPropertyValue^>(values->Lookup("ShowFPS"));
		auto enableSoundEntry = safe_cast<IPropertyValue^>(values->Lookup("SoundEnabled"));
		auto touchEnabledEntry = safe_cast<IPropertyValue^>(values->Lookup("TouchEnabled"));
		//auto lowFrequencyMode = safe_cast<IPropertyValue^>(values->Lookup("LowRefreshMode"));
		auto touchControllerOnTop = safe_cast<IPropertyValue^>(values->Lookup("TouchControllerOnTop"));
		auto controLCenterYlayer2 = safe_cast<IPropertyValue^>(values->Lookup("ControlAssignedToPlayer2"));
		auto buttonsIntersecting = safe_cast<IPropertyValue^>(values->Lookup("TouchButtonsIntersecting"));
		auto autosaving = safe_cast<IPropertyValue^>(values->Lookup("EnableAutosaving"));
		auto grayButtons = safe_cast<IPropertyValue^>(values->Lookup("GrayButtons"));
		auto aspect = safe_cast<IPropertyValue^>(values->Lookup("AspectRatio"));
		auto imageScale = safe_cast<IPropertyValue^>(values->Lookup("ImageScale"));
		auto controllerOpacity = safe_cast<IPropertyValue^>(values->Lookup("ControllerOpacity"));
		auto synchronizeAudio = safe_cast<IPropertyValue^>(values->Lookup("SynchronizeAudio"));
		auto deadzoneEntry = safe_cast<IPropertyValue^>(values->Lookup("Deadzone"));
		auto monitorTypeEntry = safe_cast<IPropertyValue^>(values->Lookup("MonitorType"));

		auto leftKeyEntry = safe_cast<IPropertyValue^>(values->Lookup("LeftKeyMapping"));
		auto rightKeyEntry = safe_cast<IPropertyValue^>(values->Lookup("RightKeyMapping"));
		auto downKeyEntry = safe_cast<IPropertyValue^>(values->Lookup("DownKeyMapping"));
		auto upKeyEntry = safe_cast<IPropertyValue^>(values->Lookup("UpKeyMapping"));
		auto lKeyEntry = safe_cast<IPropertyValue^>(values->Lookup("LKeyMapping"));
		auto rKeyEntry = safe_cast<IPropertyValue^>(values->Lookup("RKeyMapping"));
		auto startKeyEntry = safe_cast<IPropertyValue^>(values->Lookup("StartKeyMapping"));
		auto selectKeyEntry = safe_cast<IPropertyValue^>(values->Lookup("SelectKeyMapping"));
		auto bKeyEntry = safe_cast<IPropertyValue^>(values->Lookup("BKeyMapping"));
		auto aKeyEntry = safe_cast<IPropertyValue^>(values->Lookup("AKeyMapping"));
		auto xKeyEntry = safe_cast<IPropertyValue^>(values->Lookup("XKeyMapping"));
		auto yKeyEntry = safe_cast<IPropertyValue^>(values->Lookup("YKeyMapping"));
		auto turboKeyEntry = safe_cast<IPropertyValue^>(values->Lookup("TurboKeyMapping"));

		auto linearFilterEntry = safe_cast<IPropertyValue^>(values->Lookup("LinearFilter"));
		
		if(loadConfirmEntry)
		{
			disableLoadConfirmation(loadConfirmEntry->GetBoolean());
		}else
		{
			disableLoadConfirmation(DEFAULT_LOAD_CONFIRM);
		}
		if(saveConfirmEntry)
		{
			disableSaveConfirmation(saveConfirmEntry->GetBoolean());
		}else
		{
			disableSaveConfirmation(DEFAULT_SAVE_CONFIRM);
		}
		if(monitorTypeEntry)
		{
			setMonitorType(monitorTypeEntry->GetInt32());
		}else
		{
			setMonitorType(DEFAULT_MONITOR_TYPE);
		}
		if(deadzoneEntry)
		{
			setDeadzone(deadzoneEntry->GetSingle());
		}else
		{
			setDeadzone(DEFAULT_DEADZONE_VALUE);
		}

		if(synchronizeAudio)
		{
			setSynchronizeAudio(synchronizeAudio->GetBoolean());
		}else
		{
			setSynchronizeAudio(DEFAULT_SYNC_AUDIO);
		}
		if(controllerOpacity)
		{
			setControllerOpacity(controllerOpacity->GetInt32());
		}else
		{
			setControllerOpacity(DEFAULT_CONTROLLER_OPACITY);
		}


		if(aspect)
		{
			setAspectRatio((AspectRatioMode) aspect->GetInt32());
		}else
		{
			setAspectRatio(DEFAULT_ASPECT);
		}
		if(imageScale)
		{
			setImageScale(imageScale->GetInt32());
		}else
		{
			setImageScale(DEFAULT_IMAGE_SCALE);
		}
		if(autosaving)
		{
			enableAutosaving(autosaving->GetBoolean());
		}else
		{
			enableAutosaving(DEFAULT_AUTOSAVING_ENABLED);
		}
		if(touchControllerOnTop)
		{
			setTouchControllerOnTop(touchControllerOnTop->GetBoolean());
		}else
		{
			setTouchControllerOnTop(DEFAULT_TOUCH_ON_TOP);
		}
		/*if(lowFrequencyMode)
		{
			enableLowDisplayRefreshMode(lowFrequencyMode->GetBoolean());
		}*/
		if(showFpsEntry)
		{
			enableShowFPS(showFpsEntry->GetBoolean());
		}
		else
		{
			enableShowFPS(DEFAULT_SHOW_FPS_VALUE);
		}
		if(enableSoundEntry)
		{
			enableSound(enableSoundEntry->GetBoolean());
		}
		else
		{
			enableSound(DEFAULT_SOUND_ENABLED_VALUE);
		}
		if(touchEnabledEntry)
		{
			enableTouchControls(touchEnabledEntry->GetBoolean());
		}else
		{
			enableTouchControls(DEFAULT_TOUCH_ENABLED_VALUE);
		}
		if(turboKeyEntry)
		{
			setTurboKeyBinding((VirtualKey) turboKeyEntry->GetInt32());
		}else
		{
			setTurboKeyBinding((VirtualKey) DEFAULT_TURBO_KEY);
		}
		if(leftKeyEntry)
		{
			setLeftKeyBinding((VirtualKey) leftKeyEntry->GetInt32());
		}else
		{
			setLeftKeyBinding((VirtualKey) DEFAULT_LEFT_KEY);
		}
		if(rightKeyEntry)
		{
			setRightKeyBinding((VirtualKey) rightKeyEntry->GetInt32());
		}else
		{
			setRightKeyBinding((VirtualKey) DEFAULT_RIGHT_KEY);
		}
		if(upKeyEntry)
		{
			setUpKeyBinding((VirtualKey) upKeyEntry->GetInt32());
		}else
		{
			setUpKeyBinding((VirtualKey) DEFAULT_UP_KEY);
		}
		if(downKeyEntry)
		{
			setDownKeyBinding((VirtualKey) downKeyEntry->GetInt32());
		}else
		{
			setDownKeyBinding((VirtualKey) DEFAULT_DOWN_KEY);
		}
		if(startKeyEntry)
		{
			setStartKeyBinding((VirtualKey) startKeyEntry->GetInt32());
		}else
		{
			setStartKeyBinding((VirtualKey) DEFAULT_START_KEY);
		}
		if(selectKeyEntry)
		{
			setSelectKeyBinding((VirtualKey) selectKeyEntry->GetInt32());
		}else
		{
			setSelectKeyBinding((VirtualKey) DEFAULT_SELECT_KEY);
		}
		if(aKeyEntry)
		{
			setAKeyBinding((VirtualKey) aKeyEntry->GetInt32());
		}else
		{
			setAKeyBinding((VirtualKey) DEFAULT_A_KEY);
		}
		if(bKeyEntry)
		{
			setBKeyBinding((VirtualKey) bKeyEntry->GetInt32());
		}else
		{
			setBKeyBinding((VirtualKey) DEFAULT_B_KEY);
		}
		if(lKeyEntry)
		{
			setLKeyBinding((VirtualKey) lKeyEntry->GetInt32());
		}else
		{
			setLKeyBinding((VirtualKey) DEFAULT_L_KEY);
		}
		if(rKeyEntry)
		{
			setRKeyBinding((VirtualKey) rKeyEntry->GetInt32());
		}else
		{
			setRKeyBinding((VirtualKey) DEFAULT_R_KEY);
		}



		StoreSettings();
	}

}