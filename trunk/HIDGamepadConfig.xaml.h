//
// HIDGamepadConfig.xaml.h
// Declaration of the HIDGamepadConfig class
//

#pragma once

#include "HIDGamepadConfig.g.h"
#include "HIDControllerInput.h"

namespace VBA10
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class HIDGamepadConfig sealed
	{
	public:
		HIDGamepadConfig();
	protected:
		virtual void OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
	private:
		Windows::Foundation::EventRegistrationToken inputReportEventToken;
		bool isRegisteredForInputReportEvents;

		// Device that we registered for events with
		Windows::Devices::HumanInterfaceDevice::HidDevice^ registeredDevice;

		Platform::String^ currentFocus;
		Windows::UI::Xaml::Controls::TextBox^ focusTextbox;
		bool navigatedAway;

		Platform::Collections::Vector < HidNumericControlExt^>^ allNumericControls;
		bool hasHatSwitch;  //to see if the gamepad has a hat switch used as d-pad
		int configureStage; //0: press start, 1: freely assign
		int startbuttonID; //record id of start button
		//int selectbuttonID; //id of select button

		void OnInputReportEvent(
			Windows::Devices::HumanInterfaceDevice::HidDevice^ sender,
			Windows::Devices::HumanInterfaceDevice::HidInputReportReceivedEventArgs^ eventArgs);

		void txtLeft1_GotFocus(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void closeBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void txtLeft1_LostFocus(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		
	};
}
