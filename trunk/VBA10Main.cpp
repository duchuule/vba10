

#include "pch.h"
#include "VBA10Main.h"
#include "Common\DirectXHelper.h"
#include "EmulatorSettings.h"

using namespace VBA10;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
VBA10Main::VBA10Main(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: Replace this with your app's content initialization.
	try
	{
		this->emulator = new EmulatorGame(false);
		this->emulator->Initialize();

		renderer = std::unique_ptr<EmulatorRenderer>(new EmulatorRenderer(deviceResources));
		m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));
	}
	catch (std::exception &e)
	{
	#if _DEBUG
		string s(e.what());
		OutputDebugStringA(s.c_str());
	#endif
	}
	

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

VBA10Main::~VBA10Main()
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void VBA10Main::CreateWindowSizeDependentResources()
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	renderer->CreateWindowSizeDependentResources();
	renderer->CreateTransformMatrix();
}

//void VBA10Main::StartRenderLoop()
//{
//	// If the animation render loop is already running then do not start another thread.
//	if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
//	{
//		return;
//	}
//
//	// Create a task that will be run on a background thread.
//	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
//	{
//		// Calculate the updated frame and render once per vertical blanking interval.
//		while (action->Status == AsyncStatus::Started)
//		{
//			critical_section::scoped_lock lock(m_criticalSection);
//			Update();
//			if (Render())
//			{
//				m_deviceResources->Present();
//			}
//		}
//	});
//
//	// Run task on a dedicated high priority background thread.
//	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
//}
//
//void VBA10Main::StopRenderLoop()
//{
//	if (m_renderLoopWorker)
//		m_renderLoopWorker->Cancel();
//}

// Updates the application state once per frame.
void VBA10Main::Update()
{
	//process input from user 
	//ProcessInput();

	// Update scene objects.
	m_timer.Tick([&]()
	{
		// TODO: Replace this with your app's content update functions.
		renderer->Update(m_timer);
		m_fpsTextRenderer->Update(m_timer);
	});

	//renderer->Update(m_timer);
}



// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool VBA10Main::Render()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Reset the viewport to target the whole screen.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// Reset render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// Clear the back buffer and depth stencil view.
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render the scene objects.
	// TODO: Replace this with your app's content rendering functions.
	renderer->Render();
	if (ShowingFPS())
		m_fpsTextRenderer->Render();

	return true;
}


void VBA10Main::Present()
{
	m_deviceResources->Present();
}

// Notifies renderers that device resources need to be released.
void VBA10Main::OnDeviceLost()
{
	renderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void VBA10Main::OnDeviceRestored()
{
	renderer->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

