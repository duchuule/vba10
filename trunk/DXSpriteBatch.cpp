#include "pch.h"
#include "DXSpriteBatch.h"
#include "SpriteBatch_VS.h"
#include "SpriteBatch_PS.h"
#include <vector>
#include <algorithm>
#include "OnResizeBuffer.h"
#include "OnFrameBuffer.h"

using namespace std;

namespace Engine
{
	#define VERTICES_PER_SPRITE		4
	#define INDICES_PER_SPRITE		6
	#define MAX_BATCH_SIZE			2048
	#define VERTEX_SIZE				sizeof(SpriteVertex)
	#define INDEX_SIZE				sizeof(unsigned short)
	#define VERTEX_BUFFER_SIZE		MAX_BATCH_SIZE * VERTICES_PER_SPRITE * VERTEX_SIZE
	#define INDEX_BUFFER_SIZE		MAX_BATCH_SIZE * INDICES_PER_SPRITE * INDEX_SIZE
	#define NEAR_PLANE				0.001f
	#define FAR_PLANE				1000.0f
	#define DEFAULT_DEPTH			1.0f


	DXSpriteBatch::DXSpriteBatch(ID3D11Device1 *device, ID3D11DeviceContext1 *context, float width, float height)
		: device(device), context(context),
		batchedSprites(0), beginCalled(false), 
		customPS(nullptr)
	{ 
		this->queuedSprites.reserve(MAX_BATCH_SIZE);
		this->LoadShaders();
		this->InitializeBuffers();
		this->CreateStates();

		this->UpdateProjectionMatrix(width, height);
	}

	DXSpriteBatch::~DXSpriteBatch(void)
	{
		this->device = nullptr;
		this->context = nullptr;
	}

	void DXSpriteBatch::LoadShaders(void)
	{
		if(FAILED(this->device->CreateVertexShader(
			SPRITEBATCH_VS,
			sizeof(SPRITEBATCH_VS),
			NULL,
			&this->vs)))
		{

		}
		HRESULT test;
		if(FAILED(test =this->device->CreatePixelShader(
			SPRITEBATCH_PS,
			sizeof(SPRITEBATCH_PS),
			NULL,
			&this->ps)))
		{

		}
		D3D11_INPUT_ELEMENT_DESC inputElements[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		if(FAILED(this->device->CreateInputLayout(
			inputElements, 
			ARRAYSIZE(inputElements),
			SPRITEBATCH_VS,
			sizeof(SPRITEBATCH_VS),
			&this->inputLayout)))
		{

		}
	}

	void DXSpriteBatch::InitializeBuffers(void)
	{
		CD3D11_BUFFER_DESC vertexDesc(
			VERTEX_BUFFER_SIZE,
			D3D11_BIND_VERTEX_BUFFER,
			D3D11_USAGE_DYNAMIC,
			D3D11_CPU_ACCESS_WRITE);

		ComPtr<ID3D11Buffer> tmpVertexBuffer;
		if(FAILED(this->device->CreateBuffer(&vertexDesc, nullptr, tmpVertexBuffer.GetAddressOf())))
		{

		}

		vector<unsigned short> indices;
		indices.reserve(INDICES_PER_SPRITE * MAX_BATCH_SIZE);
		
		static_assert(INDICES_PER_SPRITE == 6 && VERTICES_PER_SPRITE == 4, 
			"Changing the number of indices per sprite also \
			requires changing the initial data generation for \
			the index buffer.");

		for(int i = 0; i < MAX_BATCH_SIZE * VERTICES_PER_SPRITE; i += VERTICES_PER_SPRITE)
		{
			indices.push_back(i);
			indices.push_back(i + 1);
			indices.push_back(i + 2);
			
			indices.push_back(i + 1);
			indices.push_back(i + 3);
			indices.push_back(i + 2);
		}

		D3D11_SUBRESOURCE_DATA data = {0};
		data.pSysMem = &indices.front();

		CD3D11_BUFFER_DESC indexDesc(
			INDEX_BUFFER_SIZE, 
			D3D11_BIND_INDEX_BUFFER,
			D3D11_USAGE_IMMUTABLE);
		ComPtr<ID3D11Buffer> tmpIndexBuffer;
		if(FAILED(this->device->CreateBuffer(&indexDesc, &data, tmpIndexBuffer.GetAddressOf())))
		{

		}

		CD3D11_BUFFER_DESC onResizeDesc(
			sizeof(OnResizeBuffer), 
			D3D11_BIND_CONSTANT_BUFFER,
			D3D11_USAGE_DYNAMIC,
			D3D11_CPU_ACCESS_WRITE);
		ComPtr<ID3D11Buffer> tmpOnResizeBuffer;
		if(FAILED(this->device->CreateBuffer(&onResizeDesc, nullptr, tmpOnResizeBuffer.GetAddressOf())))
		{

		}

		CD3D11_BUFFER_DESC onFrameDesc(
			sizeof(OnFrameBuffer), 
			D3D11_BIND_CONSTANT_BUFFER,
			D3D11_USAGE_DYNAMIC,
			D3D11_CPU_ACCESS_WRITE);
		ComPtr<ID3D11Buffer> tmpOnFrameBuffer;
		if(FAILED(this->device->CreateBuffer(&onFrameDesc, nullptr, tmpOnFrameBuffer.GetAddressOf())))
		{

		}

		this->onFrameBuffer = tmpOnFrameBuffer;
		this->vertexBuffer = tmpVertexBuffer;
		this->indexBuffer = tmpIndexBuffer;
		this->onResizeBuffer = tmpOnResizeBuffer;
	}

	void DXSpriteBatch::CreateStates(void)
	{
		// Blend state
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		ComPtr<ID3D11BlendState> tmpBlend;
		if(FAILED(this->device->CreateBlendState(&blendDesc, tmpBlend.GetAddressOf())))
		{

		}

		// Additive Blend State
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		ComPtr<ID3D11BlendState> tmpAdditiveBlend;
		if(FAILED(this->device->CreateBlendState(&blendDesc, tmpAdditiveBlend.GetAddressOf())))
		{

		}

		// Depth stencil state
		D3D11_DEPTH_STENCIL_DESC depthDesc;
		ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

		depthDesc.DepthEnable = true;
		depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		ComPtr<ID3D11DepthStencilState> tmpDepth;
		if(FAILED(this->device->CreateDepthStencilState(&depthDesc, tmpDepth.GetAddressOf())))
		{

		}

		// Rasterizer state
		D3D11_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.MultisampleEnable = true;

		ComPtr<ID3D11RasterizerState> tmpRasterizer;
		if(FAILED(this->device->CreateRasterizerState(&rasterizerDesc, tmpRasterizer.GetAddressOf())))
		{

		}

		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; //D3D11_FILTER_ANISOTROPIC; //D3D11_FILTER_MIN_MAG_MIP_POINT; //D3D11_FILTER_MIN_MAG_MIP_LINEAR

		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

		samplerDesc.MaxLOD = FLT_MAX;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

		ComPtr<ID3D11SamplerState> tmpSampler;
		if(FAILED(this->device->CreateSamplerState(&samplerDesc, tmpSampler.GetAddressOf())))
		{

		}

		this->alphaBlendState = tmpBlend;
		this->depthStencilState = tmpDepth;
		this->rasterizerState = tmpRasterizer;
		this->samplerState = tmpSampler;
		this->additiveBlendState = tmpAdditiveBlend;
	}

	void DXSpriteBatch::OnResize(float width, float height)
	{
		this->UpdateProjectionMatrix(width, height);
	}

	void DXSpriteBatch::UpdateProjectionMatrix(float width, float height)
	{
		OnResizeBuffer onResize;
		//this->dxdevice->GetBackbufferSize(&width, &height);
		onResize.projection = XMMatrixOrthographicOffCenterLH(0, width, height, 0, NEAR_PLANE, FAR_PLANE);

		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		if(FAILED(this->context->Map(this->onResizeBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource)))
		{

		}

		*(OnResizeBuffer *)mappedSubresource.pData = onResize;
		this->context->Unmap(this->onResizeBuffer.Get(), 0);
	}

	void DXSpriteBatch::SetCustomPixelShader(void *customPS)
	{
		this->customPS = (ID3D11PixelShader *) customPS;
	}

	void DXSpriteBatch::Begin(XMMATRIX &world)
	{
		if(beginCalled)
		{
			return;
		}
		beginCalled = true;
		this->queuedSprites.clear();
		this->batchedSprites = 0;

		this->heapSort = HeapCompareBackToFront;

		OnFrameBuffer onFrame;
		onFrame.world = world;

		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		if(FAILED(this->context->Map(this->onFrameBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource)))
		{ 
		}

		*(OnFrameBuffer *)mappedSubresource.pData = onFrame;
		this->context->Unmap(this->onFrameBuffer.Get(), 0);
	}

	void DXSpriteBatch::Draw(const Rectangle &targetArea, const Rectangle *sourceArea, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, float depth, float rotation, Color &color)
	{
		if(!beginCalled)
		{
			return;
		}
		SpriteInfo info;

		info.TargetArea = XMFLOAT4(
			(float) targetArea.X, 
			(float) targetArea.Y, 
			(float) targetArea.Width,
			(float) targetArea.Height
			);

		if(sourceArea)
		{
			info.SourceArea = XMFLOAT4(
					(float) sourceArea->X, 
					(float) sourceArea->Y, 
					(float) sourceArea->Width,
					(float) sourceArea->Height
				);
		}
		else
		{
			D3D11_TEXTURE2D_DESC desc;
			texture->GetDesc(&desc);
			info.SourceArea = XMFLOAT4A(0.0f, 0.0f, desc.Width, desc.Height);
		}

		info.Rotation = rotation;
		info.Depth = depth;
		info.Texture = texture;
		info.TextureSRV = textureSRV;
		info.Color = color;

		this->QueueSprite(info);
		
		this->FlushBatch();
	}
	
	void DXSpriteBatch::Draw(const Rectangle &target, const Rectangle *source, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, float depth, Color &color)
	{
		this->Draw(target, source, textureSRV, texture, depth, 0.0f, color);
	}

	void DXSpriteBatch::Draw(const Rectangle &target, const Rectangle *source, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, Color &color)
	{
		this->Draw(target, source, textureSRV, texture, DEFAULT_DEPTH, 0.0f, color);
	}

	void DXSpriteBatch::Draw(const Rectangle &target, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, float depth, float rotation, Color &color)
	{
		Rectangle textureBounds;
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);
		textureBounds.X = 0;
		textureBounds.Y = 0;
		textureBounds.Width = desc.Width;
		textureBounds.Height = desc.Height;
		this->Draw(target, &textureBounds, textureSRV, texture, depth, rotation, color);
	}

	void DXSpriteBatch::Draw(const Rectangle &target, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, float depth, Color &color)
	{
		this->Draw(target, textureSRV, texture, depth, 0.0f, color);
	}

	void DXSpriteBatch::Draw(const Rectangle &target, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, Color &color)
	{
		this->Draw(target, textureSRV, texture, DEFAULT_DEPTH, 0.0f, color);
	}

	void DXSpriteBatch::Draw(const Vector2 &target, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, float depth, float rotation, Color &color)
	{
		Rectangle textureBounds;
		Rectangle targetArea;
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);
		textureBounds.X = 0;
		textureBounds.Y = 0;
		targetArea.X = (int) target.X;
		targetArea.Y = (int) target.Y;
		targetArea.Width = textureBounds.Width = desc.Width;
		targetArea.Height = textureBounds.Height = desc.Height;
		this->Draw(targetArea, &textureBounds, textureSRV, texture, depth, rotation, color);
	}

	void DXSpriteBatch::Draw(const Vector2 &target, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, float depth, Color &color)
	{
		this->Draw(target, textureSRV, texture, depth, 0.0f, color);
	}

	void DXSpriteBatch::Draw(const Vector2 &target, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, Color &color)
	{
		this->Draw(target, textureSRV, texture, DEFAULT_DEPTH, 0.0f, color);
	}

	void DXSpriteBatch::QueueSprite(SpriteInfo &info)
	{
		this->queuedSprites.push_back(info);	
		this->batchedSprites++;
	}

	void DXSpriteBatch::End(void)
	{
		if(!beginCalled)
		{

		}
		this->beginCalled = false;
		if(this->batchedSprites == 0)
		{
			this->SetCustomPixelShader(nullptr);
			return;
		}

		sort_heap(this->queuedSprites.begin(), this->queuedSprites.end(), this->heapSort);

		this->FlushBatch();

		this->SetCustomPixelShader(nullptr);
	}

	void DXSpriteBatch::FlushBatch(void)
	{		
		// Put all sprites into the vertex buffer first to avoid frequent mapping and unmapping for each batch.
		D3D11_MAPPED_SUBRESOURCE mappedBuffer;
		if(FAILED(this->context->Map(this->vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer)))
		{

		}

		// Fill vertex buffer
		SpriteVertex *vertices = (SpriteVertex *) mappedBuffer.pData;
		UINT i = 0;
		for(; i < this->batchedSprites; i++)
		{
			SpriteInfo info = this->queuedSprites.at(i);
			XMVECTOR textureSize;
			XMVECTOR invTextureSize;

			D3D11_TEXTURE2D_DESC desc;
			info.Texture->GetDesc(&desc);
#if !defined(_M_ARM)
			textureSize.m128_f32[0] = desc.Width;
			textureSize.m128_f32[1] = desc.Height;
			textureSize = XMVectorSwizzle(textureSize, 0, 1, 0, 1);
			invTextureSize.m128_f32[0] = 1.0f / textureSize.m128_f32[0];
			invTextureSize.m128_f32[1] = 1.0f / textureSize.m128_f32[1];	
#else
			textureSize.n128_f32[0] = desc.Width;
			textureSize.n128_f32[1] = desc.Height;
			textureSize = XMVectorSwizzle<0,1,0,1>(textureSize);
			invTextureSize.n128_f32[0] = 1.0f / textureSize.n128_f32[0];
			invTextureSize.n128_f32[1] = 1.0f / textureSize.n128_f32[1];	
#endif
			
			invTextureSize = XMVectorSwizzle(invTextureSize,0,1,0,1);

			 // Load sprite parameters into SIMD registers.
			XMVECTOR source = XMLoadFloat4(&info.SourceArea);
			XMVECTOR destination = XMLoadFloat4(&info.TargetArea);

			float rotation = info.Rotation;
			float depth = info.Depth;

			// Extract the source and destination sizes into separate vectors.
			XMVECTOR sourceSize = XMVectorSwizzle(source, 2, 3, 2, 3);
			XMVECTOR destinationSize = XMVectorSwizzle(destination, 2, 3, 2, 3);
			XMVECTOR halfDestinationSize = destinationSize / 2.0f;
			source *= invTextureSize;
			sourceSize *= invTextureSize;

			XMVECTOR rotation1;
			XMVECTOR rotation2;

			if(rotation != 0.0f)
			{
				float sin, cos;

				XMScalarSinCos(&sin, &cos, rotation);

				XMVECTOR sinV = XMLoadFloat(&sin);
				XMVECTOR cosV = XMLoadFloat(&cos);

				rotation1 = XMVectorMergeXY(cosV, sinV);
				rotation2 = XMVectorMergeXY(-sinV, cosV);
			}else
			{
#if !defined(_M_ARM)
				rotation1.m128_f32[0] = 1.0f;
				rotation1.m128_f32[1] = 0.0f;
				rotation1.m128_f32[2] = 0.0f;
				rotation1.m128_f32[3] = 0.0f;

				rotation2.m128_f32[0] = 0.0f;
				rotation2.m128_f32[1] = 1.0f;
				rotation2.m128_f32[2] = 0.0f;
				rotation2.m128_f32[3] = 0.0f;
#else
				rotation1.n128_f32[0] = 1.0f;
				rotation1.n128_f32[1] = 0.0f;
				rotation1.n128_f32[2] = 0.0f;
				rotation1.n128_f32[3] = 0.0f;

				rotation2.n128_f32[0] = 0.0f;
				rotation2.n128_f32[1] = 1.0f;
				rotation2.n128_f32[2] = 0.0f;
				rotation2.n128_f32[3] = 0.0f;
#endif
			}

			static XMVECTORF32 corners[VERTICES_PER_SPRITE] = 
			{
				{ 0, 0 },
				{ 1, 0 },
				{ 0, 1 },
				{ 1, 1 }
			};

			// Calculate sprite vertices
			for(int j = 0; j < VERTICES_PER_SPRITE; j++)
			{
				SpriteVertex vertex;

				// Rotate the vertex around the sprites center
				XMVECTOR corner = corners[j] * destinationSize;
				corner = XMVectorSubtract(corner, halfDestinationSize); // Shift center into origin
				XMVECTOR pos1 = XMVectorMultiplyAdd(XMVectorSplatX(corner), rotation1, destination);
				XMVECTOR pos2 = XMVectorMultiplyAdd(XMVectorSplatY(corner), rotation2, XMVectorAdd(pos1, halfDestinationSize));

				XMVECTOR position = pos2;
#if !defined(_M_ARM)
				position.m128_f32[2] = NEAR_PLANE;
				position.m128_f32[3] = 1.0f;

				vertex.Position.x = position.m128_f32[0];
				vertex.Position.y = position.m128_f32[1];
				vertex.Position.z = position.m128_f32[2];
#else
				position.n128_f32[2] = NEAR_PLANE;
				position.n128_f32[3] = 1.0f;

				vertex.Position.x = position.n128_f32[0];
				vertex.Position.y = position.n128_f32[1];
				vertex.Position.z = position.n128_f32[2];
#endif

				//XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&vertex.Position), position);

				XMVECTOR texCoord = XMVectorMultiplyAdd(corners[j], sourceSize, source);

				XMStoreFloat2(&vertex.TexCoord, texCoord);

				vertex.Color = info.Color;

				*vertices = vertex;
				vertices++;
			}
		}
		this->context->Unmap(this->vertexBuffer.Get(), 0);

		// Render all sprites
		SpriteInfo lastSprite = this->queuedSprites.at(0);		
		i = 0;
		UINT batchBegin = i;
		for(; i < this->batchedSprites; i++)
		{
			SpriteInfo info = this->queuedSprites.at(i);
			
			if((info.Texture.Get() != lastSprite.Texture.Get()))
			{
				this->RenderBatch(batchBegin, i - 1, lastSprite);
				lastSprite = info;
				batchBegin = i;
			}
		}
		// Render last batch of sprites
		RenderBatch(batchBegin, i - 1, lastSprite);

		this->queuedSprites.clear();
		this->batchedSprites = 0;
	}

	void DXSpriteBatch::RenderBatch(UINT start, UINT end, SpriteInfo &spriteInfo)
	{
		ID3D11ShaderResourceView *srv = nullptr;

		srv = spriteInfo.TextureSRV.Get();		

		this->context->OMSetBlendState(this->alphaBlendState.Get(), nullptr, 0xFFFFFFFF);
		this->context->OMSetDepthStencilState(this->depthStencilState.Get(), 0);
		this->context->RSSetState(this->rasterizerState.Get());
		this->context->PSSetSamplers(0, 1, this->samplerState.GetAddressOf());

		// Shaders
		this->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		this->context->IASetInputLayout(this->inputLayout.Get());
		this->context->VSSetShader(this->vs.Get(), nullptr, 0);
		if(this->customPS)
		{
			this->context->PSSetShader(this->customPS.Get(), nullptr, 0);
		}else
		{
			this->context->PSSetShader(this->ps.Get(), nullptr, 0);
		}
		this->context->PSSetShaderResources(0, 1, &srv);

		// Buffers
		UINT stride = sizeof(SpriteVertex);
		UINT offset = 0;
		this->context->IASetVertexBuffers(0, 1, this->vertexBuffer.GetAddressOf(), &stride, &offset);
		this->context->IASetIndexBuffer(this->indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		this->context->VSSetConstantBuffers(0, 1, this->onResizeBuffer.GetAddressOf());
		this->context->VSSetConstantBuffers(1, 1, this->onFrameBuffer.GetAddressOf());

		// Draw
		this->context->DrawIndexed(
			INDICES_PER_SPRITE * (end - start + 1), 
			start * INDICES_PER_SPRITE, 0
			);
	}

	bool HeapCompareByTexture(const SpriteInfo &info1, const SpriteInfo &info2)
	{
		return true;// ((info1.ContentType == info2.ContentType) && (info1.TextureID < info2.TextureID)) ||
			//(info1.ContentType < info2.ContentType);
	}

	bool HeapCompareBackToFront(const SpriteInfo &info1, const SpriteInfo &info2)
	{
		return (info1.Depth > info2.Depth) ||
			((info1.Depth == info2.Depth) && HeapCompareByTexture(info1, info2)) ;
	}

	bool HeapCompareFrontToBack(const SpriteInfo &info1, const SpriteInfo &info2)
	{
		return (info1.Depth < info2.Depth) ||
			((info1.Depth == info2.Depth) && HeapCompareByTexture(info1, info2)) ;
	}
}