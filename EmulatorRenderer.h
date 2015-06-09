#pragma once


#include "Emulator.h"
#include "VirtualControllerInput.h"
#include "DXSpriteBatch.h"

#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"

using namespace Engine;
using namespace DirectX;
using namespace Microsoft::WRL;

namespace VBA10
{
	class EmulatorRenderer
	{
	public:
		static std::unique_ptr<EmulatorRenderer> GetInstance(void);

		EmulatorRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~EmulatorRenderer(void);

		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Render();
		void Update(DX::StepTimer const& timer);

		GameTime ^GetGameTime(void);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D Objects, no longer defined here, we moved it out to Common/DeviceResources.h
		//Microsoft::WRL::ComPtr<ID3D11Device1> m_d3dDevice;
		//Microsoft::WRL::ComPtr<ID3D11DeviceContext1> m_d3dContext;
		//Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
		//Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
		//Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;

		// Cached renderer properties, no longer defined here, we moved it to Common/DeviceResources.h
		//D3D_FEATURE_LEVEL m_featureLevel;
		//Windows::Foundation::Size m_renderTargetSize;
		//Windows::Foundation::Rect m_windowBounds;
		//Windows::UI::Core::CoreWindow ^m_window;
		//Windows::Graphics::Display::DisplayOrientations m_orientation;

		HANDLE waitEvent;

		float lastElapsed;

		//emulator variables
		float autosaveElapsed;
		float elapsedTime;
		int frames;
		int fps;
		float fpsElapsedTime;
		float elapsedTimeMeasure;
		size_t pitch;
		u8 *backbufferPtr;

		GameTime ^gameTime;
		VirtualControllerInput				*controller;
		RECT buttonsRectangle;
		RECT crossRectangle;
		RECT startSelectRectangle;
		RECT lRectangle;
		RECT rRectangle;

		int									frontbuffer;
		int									width, height;
		DXSpriteBatch						*dxSpriteBatch;
		EmulatorGame						*emulator;
		
		ComPtr<ID3D11Texture2D>				buffers[2];
		ComPtr<ID3D11ShaderResourceView>	bufferSRVs[2];
		ComPtr<ID3D11BlendState>			alphablend;
		
		ComPtr<ID3D11Resource>				stickCenterResource;
		ComPtr<ID3D11ShaderResourceView>	stickCenterSRV;
		ComPtr<ID3D11Resource>				stickResource;
		ComPtr<ID3D11ShaderResourceView>	stickSRV;
		ComPtr<ID3D11Resource>				crossResource;
		ComPtr<ID3D11ShaderResourceView>	crossSRV;
		ComPtr<ID3D11Resource>				buttonsResource;
		ComPtr<ID3D11ShaderResourceView>	buttonsSRV;
		ComPtr<ID3D11Resource>				startSelectResource;
		ComPtr<ID3D11ShaderResourceView>	startSelectSRV;
		ComPtr<ID3D11Resource>				lButtonResource;
		ComPtr<ID3D11ShaderResourceView>	lButtonSRV;
		ComPtr<ID3D11Resource>				rButtonResource;
		ComPtr<ID3D11ShaderResourceView>	rButtonSRV;
		
		void AutosaveAsync(void);
		void *MapBuffer(int index, size_t *rowPitch);
		void FPSCounter(void);
		//void MeasureTime(void);
		void Autosave(void);





	};
}