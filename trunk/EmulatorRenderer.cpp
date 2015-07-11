#include "pch.h"
#include "..\Common\DirectXHelper.h"
#include "EmulatorRenderer.h"
#include "EmulatorFileHandler.h"
#include "EmulatorSettings.h"
#include <string>
#include <sstream>
#include "Vector4.h"
#include "TextureLoader.h"


#define CROSS_TEXTURE_FILE_NAME						L"Assets/Direct3D/pad_cross.dds"
#define START_TEXTURE_FILE_NAME						L"Assets/Direct3D/pad_start.dds"
#define SELECT_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_select.dds"
#define TURBO_TEXTURE_FILE_NAME						L"Assets/Direct3D/pad_turbo_button.dds"
#define COMBO_TEXTURE_FILE_NAME						L"Assets/Direct3D/pad_combo_button.dds"
#define A_TEXTURE_FILE_NAME							L"Assets/Direct3D/pad_a_button.dds"
#define B_TEXTURE_FILE_NAME							L"Assets/Direct3D/pad_b_button.dds"
#define L_TEXTURE_FILE_NAME							L"Assets/Direct3D/pad_l_button.dds"
#define R_TEXTURE_FILE_NAME							L"Assets/Direct3D/pad_r_button.dds"
#define STICK_TEXTURE_FILE_NAME						L"Assets/Direct3D/thumbstick.dds"
#define STICK_CENTER_TEXTURE_FILE_NAME				L"Assets/Direct3D/thumbstickcenter.dds"

#define CROSS_COLOR_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_cross_color.dds"
#define START_COLOR_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_start_color.dds"
#define SELECT_COLOR_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_select_color.dds"
#define TURBO_COLOR_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_turbo_button_color.dds"
#define COMBO_COLOR_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_combo_button_color.dds"
#define A_COLOR_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_a_button_color.dds"
#define B_COLOR_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_b_button_color.dds"
#define L_COLOR_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_l_button_color.dds"
#define R_COLOR_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_r_button_color.dds"
#define STICK_COLOR_TEXTURE_FILE_NAME				L"Assets/Direct3D/thumbstick_color.dds"

#define CROSS_GBASP_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_cross_gbasp.dds"
#define START_GBASP_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_start_gbasp.dds"
#define SELECT_GBASP_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_select_gbasp.dds"
#define TURBO_GBASP_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_turbo_button_gbasp.dds"
#define COMBO_GBASP_TEXTURE_FILE_NAME				L"Assets/Direct3D/pad_combo_button_gbasp.dds"
#define A_GBASP_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_a_button_gbasp.dds"
#define B_GBASP_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_b_button_gbasp.dds"
#define L_GBASP_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_l_button_gbasp.dds"
#define R_GBASP_TEXTURE_FILE_NAME					L"Assets/Direct3D/pad_r_button_gbasp.dds"
#define STICK_GBASP_TEXTURE_FILE_NAME				L"Assets/Direct3D/thumbstick_gbasp.dds"

#define DIVIDER_TEXTURE_FILE_NAME					L"Assets/Direct3D/divider.dds"


#define AUTOSAVE_INTERVAL				10.0f

using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

HANDLE swapEvent = NULL;
HANDLE updateEvent = NULL;

bool lastSkipped = false;

int framesNotRendered = 0;
bool monitorTypeSkipped = false;

void ContinueEmulation(void)
{
	if(swapEvent && updateEvent)
	{
		ResetEvent(swapEvent);
		SetEvent(updateEvent);
	}
}

extern u8 *pix;
size_t gbaPitch;

inline void cpyImg32( unsigned char *dst, unsigned int dstPitch, unsigned char *src, unsigned int srcPitch, unsigned short width, unsigned short height )
{
	// fast, iterative C version
	// copies an width*height array of visible pixels from src to dst
	// srcPitch and dstPitch are the number of garbage bytes after a scanline
	register unsigned short lineSize = width<<2;

	while( height-- ) {
		memcpy( dst, src, lineSize );
		src += srcPitch;
		dst += dstPitch;
	}
}

namespace VBA10
{
	//extern bool timeMeasured;
	extern bool autosaving;
	extern bool gbaROMLoaded;


	EmulatorRenderer::EmulatorRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources)
		: emulator(EmulatorGame::GetInstance()),
		frontbuffer(0), controller(nullptr),
		elapsedTime(0.0f), frames(0), autosaveElapsed(0.0f),
		m_deviceResources(deviceResources)
	{ 
		this->gameTime = ref new GameTime();
		this->waitEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);

		swapEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
		updateEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);

		/*this->stopThread = false;
		this->autosaveDoneEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
		this->autosaveEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
		this->threadAction = ThreadPool::RunAsync(ref new WorkItemHandler([this](IAsyncAction ^action)
		{
		this->AutosaveAsync();
		}), WorkItemPriority::High, WorkItemOptions::None);*/


		//m_window = CoreWindow::GetForCurrentThread();



		//load textures and similar stuff 
		CreateDeviceDependentResources();
		CreateWindowSizeDependentResources();

		this->controller = this->emulator->GetVirtualController();


		//this->emulator->ResizeBuffer(m_deviceResources->GetOutputSize().Width, m_deviceResources->GetOutputSize().Height);
		//

		//this->width = m_deviceResources->GetOutputSize().Width;
		//this->height = m_deviceResources->GetOutputSize().Height;

		//if (!this->dxSpriteBatch)
		//{
		//	this->dxSpriteBatch = new DXSpriteBatch(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext(), this->width, this->height);
		//}
		//else
		//{
		//	this->dxSpriteBatch->OnResize(this->width, this->height);
		//}
	}

	EmulatorRenderer::~EmulatorRenderer(void)
	{
		if(m_deviceResources->GetD3DDeviceContext())
		{
			m_deviceResources->GetD3DDeviceContext()->Unmap(this->buffers[(this->frontbuffer + 1) % 2].Get(), 0);
		}

		CloseHandle(this->waitEvent);
		CloseHandle(swapEvent);
		CloseHandle(updateEvent);

		/*this->stopThread = true;
		SetEvent(this->autosaveEvent);
		WaitForSingleObjectEx(this->autosaveDoneEvent, INFINITE, false);*/
		
		delete this->dxSpriteBatch;
		this->dxSpriteBatch = nullptr;
	}



	GameTime ^EmulatorRenderer::GetGameTime(void)
	{
		return this->gameTime;
	}


	void EmulatorRenderer::CreateDeviceDependentResources()
	{
		//this does not seem neccessary 
		m_deviceResources->GetD3DDevice()->GetImmediateContext2(m_deviceResources->GetD3DDeviceContextAddress());
		
		LoadTextureFromFile(
			m_deviceResources->GetD3DDevice(),
			DIVIDER_TEXTURE_FILE_NAME,
			this->dividerResource.GetAddressOf(),
			this->dividerSRV.GetAddressOf()
			);

		LoadTextureFromFile(
			m_deviceResources->GetD3DDevice(),
			STICK_GBASP_TEXTURE_FILE_NAME,
			this->stickResource.GetAddressOf(), 
			this->stickSRV.GetAddressOf()
			);
		
		LoadTextureFromFile(
			m_deviceResources->GetD3DDevice(),
			STICK_CENTER_TEXTURE_FILE_NAME,
			this->stickCenterResource.GetAddressOf(), 
			this->stickCenterSRV.GetAddressOf()
			);
		
		LoadTextureFromFile(
			m_deviceResources->GetD3DDevice(),
			CROSS_GBASP_TEXTURE_FILE_NAME,
			this->crossResource.GetAddressOf(), 
			this->crossSRV.GetAddressOf()
			);
		
		LoadTextureFromFile(
			m_deviceResources->GetD3DDevice(),
			A_GBASP_TEXTURE_FILE_NAME,
			this->aResource.GetAddressOf(), 
			this->aSRV.GetAddressOf()
			);

		LoadTextureFromFile(
			m_deviceResources->GetD3DDevice(),
			B_GBASP_TEXTURE_FILE_NAME,
			this->bResource.GetAddressOf(),
			this->bSRV.GetAddressOf()
			);
		
		LoadTextureFromFile(
			m_deviceResources->GetD3DDevice(),
			START_GBASP_TEXTURE_FILE_NAME,
			this->startResource.GetAddressOf(), 
			this->startSRV.GetAddressOf()
			);

		LoadTextureFromFile(
			m_deviceResources->GetD3DDevice(),
			SELECT_GBASP_TEXTURE_FILE_NAME,
			this->selectResource.GetAddressOf(),
			this->selectSRV.GetAddressOf()
			);

		LoadTextureFromFile(
			m_deviceResources->GetD3DDevice(),
			TURBO_GBASP_TEXTURE_FILE_NAME,
			this->turboResource.GetAddressOf(),
			this->turboSRV.GetAddressOf()
			);

		LoadTextureFromFile(
			m_deviceResources->GetD3DDevice(),
			COMBO_GBASP_TEXTURE_FILE_NAME,
			this->comboResource.GetAddressOf(),
			this->comboSRV.GetAddressOf()
			);
		
		LoadTextureFromFile(
			m_deviceResources->GetD3DDevice(),
			L_GBASP_TEXTURE_FILE_NAME,
			this->lButtonResource.GetAddressOf(), 
			this->lButtonSRV.GetAddressOf()
			);
		
		LoadTextureFromFile(
			m_deviceResources->GetD3DDevice(),
			R_GBASP_TEXTURE_FILE_NAME,
			this->rButtonResource.GetAddressOf(), 
			this->rButtonSRV.GetAddressOf()
			);

		// Create Textures and SRVs for front and backbuffer
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		//desc.Format = DXGI_FORMAT_B5G6R5_UNORM;
		desc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
		desc.Width = 241;//EXT_WIDTH;
		desc.Height = 162;//EXT_HEIGHT;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DYNAMIC;

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateTexture2D(&desc, nullptr, this->buffers[0].GetAddressOf())
			);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateTexture2D(&desc, nullptr, this->buffers[1].GetAddressOf())
			);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateShaderResourceView(this->buffers[0].Get(), &srvDesc, this->bufferSRVs[0].GetAddressOf())
			);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateShaderResourceView(this->buffers[1].Get(), &srvDesc, this->bufferSRVs[1].GetAddressOf())
			);

		// Map backbuffer so it can be unmapped on first update
		int backbuffer = (this->frontbuffer + 1) % 2;
		this->backbufferPtr = (uint8 *) this->MapBuffer(backbuffer, &this->pitch);
		pix = this->backbufferPtr;

		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBlendState(&blendDesc, this->alphablend.GetAddressOf())
			);
	}

	void EmulatorRenderer::CreateWindowSizeDependentResources()
	{
		//float scale = ((int)Windows::Graphics::Display::DisplayProperties::ResolutionScale) / 100.0f;
		this->width = m_deviceResources->GetOutputSize().Width;// width * scale;
		this->height = m_deviceResources->GetOutputSize().Height;//height * scale;

		this->renderwidth = m_deviceResources->GetRenderTargetSize().Width;
		this->renderheight = m_deviceResources->GetRenderTargetSize().Height;  

		//resize buffers take the render width and height, regardless of orientation
		if(!this->dxSpriteBatch)
		{
			this->dxSpriteBatch = new DXSpriteBatch(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext(), this->renderwidth, this->renderheight);
		}else
		{
			this->dxSpriteBatch->OnResize(this->renderwidth, this->renderheight);
		}

		
		this->emulator->ResizeBuffer(this->width, this->height);
	}

	void EmulatorRenderer::ReleaseDeviceDependentResources()
	{
		// Reset these member variables to ensure that UpdateForWindowSizeChange recreates all resources.
		//NOTE: may not be correct
		//m_windowBounds.Width = 0;
		//m_windowBounds.Height = 0;
		//m_swapChain = nullptr;
	}


	void EmulatorRenderer::FPSCounter(void)
	{
		float lastElapsed = this->gameTime->GetLastFrameElapsed();
		this->fpsElapsedTime += lastElapsed;
		if(this->fpsElapsedTime >= 1.0)
		{
			this->fps = this->frames;
			this->frames = 0;
			this->fpsElapsedTime -= 1.0;
		}
	}

	/*void EmulatorRenderer::MeasureTime(void)
	{
		float lastElapsed = this->gameTime->GetLastFrameElapsed();
		this->elapsedTimeMeasure += lastElapsed;
		if(this->elapsedTimeMeasure >= 3.0)
		{
			timeMeasured = true;
			if(this->fps < 34)
			{
				EnableLowDisplayRefreshMode(true);
			}
		}
	}*/

	void EmulatorRenderer::AutosaveAsync(void)
	{
		/*WaitForSingleObjectEx(this->autosaveEvent, INFINITE, false);
		while(!this->stopThread)
		{
		this->emulator->Pause();
		SaveSRAMAsync().wait();
		int oldSlot = SavestateSlot;
		SavestateSlot = AUTOSAVESTATE_SLOT;
		SaveStateAsync().wait();
		SavestateSlot = oldSlot;
		Settings.Mute = !SoundEnabled();

		SetEvent(this->autosaveDoneEvent);
		WaitForSingleObjectEx(this->autosaveEvent, INFINITE, false);
		}
		SetEvent(this->autosaveDoneEvent);*/
	}

	void EmulatorRenderer::Autosave(void)
	{
		if(AutosavingEnabled() && !this->emulator->IsPaused() && IsROMLoaded())
		{
			float lastElapsed = this->gameTime->GetLastFrameElapsed();
			this->autosaveElapsed += lastElapsed;
			if(this->autosaveElapsed >= AUTOSAVE_INTERVAL)
			{
				this->autosaveElapsed -= AUTOSAVE_INTERVAL;

				create_task([this]()
				{
					SaveSRAMCopyAsync();
				});
			}
		}
	}

	void EmulatorRenderer::Update(DX::StepTimer const& timer)
	{
		this->gameTime->Update();

		float timeDelta = this->gameTime->GetLastFrameElapsed();


		/*if(!timeMeasured)
		{
			this->MeasureTime();
		}*/

		if(!emulator->IsPaused())
		{			
			this->elapsedTime += timeDelta;

			this->lastElapsed = timeDelta;
			
			systemFrameSkip = GetPowerFrameSkip();
			float targetFPS = 55.0f;
			if(GetMonitorType() == 0)
			{
				systemFrameSkip = systemFrameSkip * 2 + 1;
				targetFPS = 28.0f;
			}
			if(GetFrameSkip() == -1 && GetPowerFrameSkip() == 0)
			{
				if(!lastSkipped && (this->lastElapsed * 1.0f) > (1.0f / targetFPS))
				{
					int skip = (int)((this->lastElapsed * 1.0f) / (1.0f / targetFPS));				
					systemFrameSkip += (skip < 2) ? skip : 2;
					//systemFrameSkip++;
					lastSkipped = true;
				}else
				{
					lastSkipped = false;
				}
			}else if(GetFrameSkip() >= 0)
			 {
				systemFrameSkip += GetFrameSkip();
			 }

			this->Autosave();

			this->emulator->Update();
		}
		this->FPSCounter();
	}


	void EmulatorRenderer::Render()
	{
		m_deviceResources->GetD3DDeviceContext()->OMSetRenderTargets(
			1,
			m_deviceResources->GetBackBufferRenderTargetViewAddress(),
			m_deviceResources->GetDepthStencilView()
			);

		float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.000f }; //black
		if (this->height > this->width) //portrait
		{
			bgcolor[0] = 210.0f / 255;
			bgcolor[1] = 210.0f / 255;
			bgcolor[2] = 210.0f / 255;
		}

		m_deviceResources->GetD3DDeviceContext()->ClearRenderTargetView(
			m_deviceResources->GetBackBufferRenderTargetView(),
			bgcolor
			);

		m_deviceResources->GetD3DDeviceContext()->ClearDepthStencilView(
			m_deviceResources->GetDepthStencilView(),
			D3D11_CLEAR_DEPTH,
			1.0f,
			0
			);

		if(!this->emulator->IsPaused()/* || autosaving*/)
		{
			if((GetMonitorType() != MONITOR_120HZ) || (monitorTypeSkipped))
			{
				monitorTypeSkipped = false;
				if(framesNotRendered >= GetPowerFrameSkip())
				{
					framesNotRendered = 0;
					WaitForSingleObjectEx(swapEvent, INFINITE, false);

					int backbuffer = this->frontbuffer;
					this->frontbuffer = (this->frontbuffer + 1) % 2;

					uint8 *buffer = (uint8 *) this->MapBuffer(backbuffer, &gbaPitch);
					this->backbufferPtr = buffer;
					this->pitch = gbaPitch;

					pix = buffer;

					m_deviceResources->GetD3DDeviceContext()->Unmap(this->buffers[this->frontbuffer].Get(), 0);

					SetEvent(updateEvent);
				}else
				{
					framesNotRendered++;
				}
			}else
			{
				monitorTypeSkipped = true;
			}
		}

		int height;
		int width;
		RECT rect;


		height = this->height; // *(GetImageScale() / 100.0f);
		switch (GetAspectRatio())
		{
		default:
		case AspectRatioMode::Original:
			if (gbaROMLoaded)
			{
				width = (int)(height * (240.0f / 160.0f));
			}
			else
			{
				width = (int)(height * (160.0f / 144.0f));
			}
			break;
		case AspectRatioMode::Stretch:
			width = this->width; //* (GetImageScale() / 100.0f);
			break;
		case AspectRatioMode::FourToThree:
			width = (int)(height * (4.0f / 3.0f));
			break;
		case AspectRatioMode::FiveToFour:
			width = (int)(height * (5.0f / 4.0f));
			break;
		case AspectRatioMode::One:
			width = height;
			break;
		}

		if (width > this->width) //fix the position of the image
		{
			height = height * 1.0f / width * this->width;
			width = this->width;
		}

		int leftOffset = (this->width - width) / 2;
		rect.left = leftOffset;
		rect.right = width + leftOffset;
		rect.top = 0;
		rect.bottom = height;

		RECT dividerRect = RECT();
		if (this->height > this->width) //portrait
		{
			dividerRect.left = 0;
			dividerRect.right = width;
			dividerRect.top = height;
			float scale = DisplayInformation::GetForCurrentView()->RawPixelsPerViewPixel;
			dividerRect.bottom = height + 4 * scale;
		}

		RECT source;
		if(gbaROMLoaded)
		{
			source.left = 0;
			source.right = 240;
			source.top = 2;
			source.bottom = 161;
		}else
		{
			source.left = 0;
			source.right = 160;
			source.top = 2;
			source.bottom = 144;
		}

		this->controller->GetARectangle(&aRectangle);
		this->controller->GetBRectangle(&bRectangle);
		this->controller->GetCrossRectangle(&crossRectangle);
		this->controller->GetStartRectangle(&startRectangle);
		this->controller->GetSelectRectangle(&selectRectangle);
		this->controller->GetTurboRectangle(&turboRectangle);
		this->controller->GetComboRectangle(&comboRectangle);
		this->controller->GetLRectangle(&lRectangle);
		this->controller->GetRRectangle(&rRectangle);

		XMFLOAT4A colorf = XMFLOAT4A(1.0f, 1.0f, 1.0f, GetControllerOpacity() / 100.0f);
		XMFLOAT4A colorf2 = XMFLOAT4A(1.0f, 1.0f, 1.0f, (GetControllerOpacity() / 100.0f) + 0.2f);
		XMVECTOR colorv = XMLoadFloat4A(&colorf);
		XMVECTOR colorv2 = XMLoadFloat4A(&colorf2);
		
		// Render last frame to screen
		Color white(1.0f, 1.0f, 1.0f, 1.0f);

		Color color(1.0f, 1.0f, 1.0f, 1.0f);
		if (this->width > this->height) //landscape
			color = Color(1.0f, 1.0f, 1.0f, 0.5f);
		

		

		XMMATRIX x = XMLoadFloat4x4(&this->outputTransform);

		this->dxSpriteBatch->Begin(x, EmulatorSettings::Current->LinearFilterEnabled);

		Engine::Rectangle sourceRect(source.left, source.top, source.right - source.left, source.bottom - source.top);
		Engine::Rectangle targetRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);

		this->dxSpriteBatch->Draw(targetRect, &sourceRect, this->bufferSRVs[this->frontbuffer].Get(), this->buffers[this->frontbuffer].Get(), white);

		if(TouchControlsEnabled())
		{
			int dpad = EmulatorSettings::Current->DPadStyle;
			if(dpad == 0)
			{
				Engine::Rectangle crossRect (this->crossRectangle.left, this->crossRectangle.top, this->crossRectangle.right - this->crossRectangle.left, this->crossRectangle.bottom - this->crossRectangle.top);

				ComPtr<ID3D11Texture2D> tex;
				this->crossResource.As(&tex);
				this->dxSpriteBatch->Draw(crossRect, this->crossSRV.Get(), tex.Get(), color);
			}
			else if(dpad == 1 || (dpad == 2 && this->controller->StickFingerDown()))
			{
				RECT centerRect;
				RECT stickRect;
				this->controller->GetStickRectangle(&stickRect);
				this->controller->GetStickCenterRectangle(&centerRect);

				Engine::Rectangle stickRectE (stickRect.left, stickRect.top, stickRect.right - stickRect.left, stickRect.bottom - stickRect.top);
				Engine::Rectangle stickRectCenterE (centerRect.left, centerRect.top, centerRect.right - centerRect.left, centerRect.bottom - centerRect.top);

				ComPtr<ID3D11Texture2D> tex;
				this->stickResource.As(&tex);
				ComPtr<ID3D11Texture2D> tex2;
				this->stickCenterResource.As(&tex2);
				this->dxSpriteBatch->Draw(stickRectCenterE, this->stickCenterSRV.Get(), tex2.Get(), color);
				this->dxSpriteBatch->Draw(stickRectE, this->stickSRV.Get(), tex.Get(), color);
			}

			//divider
			Color dividerColor(86.0f / 255, 105.0f / 255, 108.0f / 255, 1.0f);
			if (this->height > this->width)
			{
				ComPtr<ID3D11Texture2D> tex;
				this->dividerResource.As(&tex);

				Engine::Rectangle targetRect(dividerRect.left, dividerRect.top, dividerRect.right - dividerRect.left, dividerRect.bottom - dividerRect.top);
				this->dxSpriteBatch->Draw(targetRect, this->dividerSRV.Get(), tex.Get(), dividerColor);
			}

			Engine::Rectangle aRect(this->aRectangle.left, this->aRectangle.top, this->aRectangle.right - this->aRectangle.left, this->aRectangle.bottom - this->aRectangle.top);
			ComPtr<ID3D11Texture2D> tex;
			this->aResource.As(&tex);
			this->dxSpriteBatch->Draw(aRect, this->aSRV.Get(), tex.Get(), color);

			Engine::Rectangle bRect(this->bRectangle.left, this->bRectangle.top, this->bRectangle.right - this->bRectangle.left, this->bRectangle.bottom - this->bRectangle.top);
			ComPtr<ID3D11Texture2D> tex2;
			this->bResource.As(&tex2);
			this->dxSpriteBatch->Draw(bRect, this->bSRV.Get(), tex2.Get(), color);


			Engine::Rectangle startRectE (startRectangle.left, startRectangle.top, startRectangle.right - startRectangle.left, startRectangle.bottom - startRectangle.top);
			ComPtr<ID3D11Texture2D> texStart;
			this->startResource.As(&texStart);
			this->dxSpriteBatch->Draw(startRectE, this->startSRV.Get(), texStart.Get(), color);

			Engine::Rectangle selectRectE(selectRectangle.left, selectRectangle.top, selectRectangle.right - selectRectangle.left, selectRectangle.bottom - selectRectangle.top);
			ComPtr<ID3D11Texture2D> texSelect;
			this->selectResource.As(&texSelect);
			this->dxSpriteBatch->Draw(selectRectE, this->selectSRV.Get(), texSelect.Get(), color);

			Engine::Rectangle turboRectE(turboRectangle.left, turboRectangle.top, turboRectangle.right - turboRectangle.left, turboRectangle.bottom - turboRectangle.top);
			ComPtr<ID3D11Texture2D> texTurbo;
			this->turboResource.As(&texTurbo);
			this->dxSpriteBatch->Draw(turboRectE, this->turboSRV.Get(), texTurbo.Get(), color);

			Engine::Rectangle comboRectE(comboRectangle.left, comboRectangle.top, comboRectangle.right - comboRectangle.left, comboRectangle.bottom - comboRectangle.top);
			ComPtr<ID3D11Texture2D> texCombo;
			this->comboResource.As(&texCombo);
			this->dxSpriteBatch->Draw(comboRectE, this->comboSRV.Get(), texCombo.Get(), color);

			if(gbaROMLoaded)
			{
				Engine::Rectangle lRectE (lRectangle.left, lRectangle.top, lRectangle.right - lRectangle.left, lRectangle.bottom - lRectangle.top);
				Engine::Rectangle rRectE (rRectangle.left, rRectangle.top, rRectangle.right - rRectangle.left, rRectangle.bottom - rRectangle.top);

				ComPtr<ID3D11Texture2D> tex;
				this->lButtonResource.As(&tex);
				ComPtr<ID3D11Texture2D> tex2;
				this->rButtonResource.As(&tex2);
				this->dxSpriteBatch->Draw(lRectE, this->lButtonSRV.Get(), tex.Get(), color);
				this->dxSpriteBatch->Draw(rRectE, this->rButtonSRV.Get(), tex2.Get(), color);
			}
		}

		/*if(ShowingFPS())
		{
			wstringstream wss;
			wss << L"FPS: ";
			wss << this->fps;

			XMFLOAT4A fpscolor = XMFLOAT4A(1.0f, 1.0f, 1.0f, 1.0f);
			XMVECTOR fpscolorA = XMLoadFloat4A(&fpscolor);

			XMFLOAT2A pos = XMFLOAT2A(10.0f, 10.0f);
			XMVECTOR posA = XMLoadFloat2A(&pos);

			XMFLOAT2A origin = XMFLOAT2A(0.0f, 0.0f);
			XMVECTOR originA = XMLoadFloat2A(&origin);

			this->font->DrawString(this->spriteBatch, wss.str().c_str(), posA, fpscolorA, 0.0f, originA, 1.0f);
		}*/

		this->dxSpriteBatch->End();

		frames++;
	}

	void *EmulatorRenderer::MapBuffer(int index, size_t *rowPitch)
	{
		D3D11_MAPPED_SUBRESOURCE map;
		ZeroMemory(&map, sizeof(D3D11_MAPPED_SUBRESOURCE));

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDeviceContext()->Map(this->buffers[index].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)
			);

		*rowPitch = map.RowPitch;
		return map.pData;
	}

	void EmulatorRenderer::GetBackbufferData(uint8 **backbufferPtr, size_t *pitch, int *imageWidth, int *imageHeight)
	{
		*backbufferPtr = this->backbufferPtr + this->pitch;
		*pitch = this->pitch;
		if (gbaROMLoaded)
		{
			*imageWidth = 240;
			*imageHeight = 160;
		}
		else
		{
			*imageWidth = 160;
			*imageHeight = 144;
		}
	}

	void EmulatorRenderer::CreateTransformMatrix(void)
	{


		if (m_deviceResources->GetRotation() == DXGI_MODE_ROTATION_IDENTITY)
		{
			XMStoreFloat4x4(&this->outputTransform, XMMatrixIdentity());
		}
		else if (m_deviceResources->GetRotation() == DXGI_MODE_ROTATION_ROTATE180)
		{
			XMStoreFloat4x4(&this->outputTransform, XMMatrixMultiply(XMMatrixRotationZ(XM_PI), XMMatrixTranslation(this->renderwidth, this->renderheight, 0.0f)));
		}
		else if (m_deviceResources->GetRotation() == DXGI_MODE_ROTATION_ROTATE90)
		{

			XMStoreFloat4x4(&this->outputTransform, XMMatrixMultiply(XMMatrixRotationZ(XM_PIDIV2), XMMatrixTranslation(this->renderwidth, 0.0f, 0.0f)));

		}
		else if (m_deviceResources->GetRotation() == DXGI_MODE_ROTATION_ROTATE270)
		{
			XMStoreFloat4x4(&this->outputTransform, XMMatrixMultiply(XMMatrixRotationZ(-XM_PIDIV2), XMMatrixTranslation(0.0f, this->renderheight, 0.0f)));
		}
	}


}

void systemDrawScreen() 
{ 
	///*EnterCriticalSection(&swapCS);

	//cpyImg32(tmpBuf + 964, 964, pix + 964, 964, 240, 160);

	//LeaveCriticalSection(&swapCS);*/

	LeaveCriticalSection(&pauseSync);

	SetEvent(swapEvent);

	WaitForSingleObjectEx(updateEvent, INFINITE, false);

	EnterCriticalSection(&pauseSync);
}