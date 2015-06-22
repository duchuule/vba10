#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "SampleFpsTextRenderer.h"
#include "EmulatorRenderer.h"
#include "Emulator.h"

// Renders Direct2D and 3D content on the screen.
namespace VBA10
{
	class VBA10Main : public DX::IDeviceNotify
	{
	public:
		VBA10Main(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~VBA10Main();
		void CreateWindowSizeDependentResources();

		// TODO: Replace with your own content renderers.

		std::unique_ptr<EmulatorRenderer> renderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;

		//void StartRenderLoop();
		//void StopRenderLoop();
		Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

		//variables
		EmulatorGame *emulator;

		//functions
		void Update();
		bool Render();
		void Present();

	private:

		

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		

		Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;

		// Rendering loop timer.
		DX::StepTimer m_timer;


	};
}