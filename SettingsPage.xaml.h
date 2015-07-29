//
// SettingsPage.xaml.h
// Declaration of the SettingsPage class
//

#pragma once

#include "SettingsPage.g.h"
#include "Emulator.h"


namespace VBA10
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class SettingsPage sealed
	{
	public:
		SettingsPage();

	private:
		EmulatorGame *emulator;
		bool initdone;

		//input
		void watchVideobtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void UpdateTextBox(Windows::UI::Xaml::Controls::TextBox ^box, VirtualKey vk);
		void touchToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void keyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void leftKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void upKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void rightKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void downKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void bKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void aKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void lKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void rKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void startKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void selectKeyBox_KeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void assignControlToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void controllerScaleSlider_ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void buttonScaleSlider_ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void turboModeBox_KeyUp_1(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void controllerOpacitySlider_ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);
		void dpadComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void deadzoneSlider_ValueChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);

		//video
		void enableTurboToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void fpsToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		//void skipComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void turboSkipComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void aspectComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void monitorComboBox_SelectionChanged_1(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		

		//general
		void saveConfirmationToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void loadConfirmationToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void soundToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void soundSyncToggle_Toggled_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void linearFilterToggle_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		
	};
}
