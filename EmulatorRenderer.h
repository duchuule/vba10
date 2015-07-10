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

		void GetBackbufferData(uint8 **backbufferPtr, size_t *pitch, int *imageWidth, int *imageHeight);
		void CreateTransformMatrix();
	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;


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
		RECT aRectangle;
		RECT bRectangle;
		RECT crossRectangle;
		RECT startRectangle;
		RECT selectRectangle;
		RECT lRectangle;
		RECT rRectangle;

		int									frontbuffer;
		int									width, height;
		int									renderwidth, renderheight;
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
		ComPtr<ID3D11Resource>				aResource;
		ComPtr<ID3D11ShaderResourceView>	aSRV;
		ComPtr<ID3D11Resource>				bResource;
		ComPtr<ID3D11ShaderResourceView>	bSRV;
		ComPtr<ID3D11Resource>				startResource;
		ComPtr<ID3D11ShaderResourceView>	startSRV;
		ComPtr<ID3D11Resource>				selectResource;
		ComPtr<ID3D11ShaderResourceView>	selectSRV;
		ComPtr<ID3D11Resource>				lButtonResource;
		ComPtr<ID3D11ShaderResourceView>	lButtonSRV;
		ComPtr<ID3D11Resource>				rButtonResource;
		ComPtr<ID3D11ShaderResourceView>	rButtonSRV;

		XMFLOAT4X4							outputTransform;
		
		void AutosaveAsync(void);
		void *MapBuffer(int index, size_t *rowPitch);
		void FPSCounter(void);
		//void MeasureTime(void);
		void Autosave(void);
		




	};
}