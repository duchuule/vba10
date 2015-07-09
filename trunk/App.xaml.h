//
// App.xaml.h
// Declaration of the App class.
//

#pragma once

#include "App.g.h"
#include "DirectXPage.xaml.h"
#include "Database\ROMDatabase.h"
#include "EmulatorSettings.h"

namespace VBA10
{
		/// <summary>
	/// Provides application-specific behavior to supplement the default Application class.
	/// </summary>
	ref class App sealed
	{
	public:
		App();
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;

	internal:
		static ROMDatabase^ ROMDB;
		

	private:
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnResuming(Platform::Object ^sender, Platform::Object ^args);
		DirectXPage^ m_directXPage;


	};
}
