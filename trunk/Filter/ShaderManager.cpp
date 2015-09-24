#include "ShaderManager.h"
#include "../TextureLoader.h"
#include "xBR2_PS.h"
#include "xBR5_PS.h"
#include "hq2x_PS.h"

using namespace VBA10;
using namespace Microsoft::WRL;

ShaderManager *ShaderManager::instance = nullptr;

ShaderManager::ShaderManager(ID3D11Device1 *device, ID3D11DeviceContext1 *context):device(device), context(context)
{
	ShaderManager::instance = this;
}

ShaderManager *ShaderManager::GetInstance()
{
	return ShaderManager::instance;
}

void ShaderManager::LoadShader(int selection)
{
	HRESULT test;
	
	//load the Look up table (texture)
	LoadTextureFromFile(
		this->device.Get(),
		L"Filter/hq2x.dds",
		this->lutResource.GetAddressOf(),
		this->lutSRV.GetAddressOf()
		);


	//create sampler
	//D3D11_SAMPLER_DESC samplerDesc;
	//ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

	//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	//samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	//samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	//samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	//samplerDesc.MaxLOD = FLT_MAX;
	//samplerDesc.MaxAnisotropy = 1;
	//samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	//if (FAILED(this->device->CreateSamplerState(&samplerDesc, this->sampler.GetAddressOf())))
	//{
	//	int test = 1;
	//}

	//this->context->PSSetSamplers(1, 1, this->sampler.GetAddressOf());


	if (selection == 1)
	{
		if (FAILED(test = this->device->CreatePixelShader(
			HQ2X_PS,
			sizeof(HQ2X_PS),
			NULL,
			&this->ps)))
		{
		}

		

	}
	else if (selection == 2)  //2xbr
	{
		if (FAILED(test = this->device->CreatePixelShader(
			XBR2_PS,
			sizeof(XBR2_PS),
			NULL,
			&this->ps)))
		{
		}
	}
	else if (selection == 3)  //5xbr
	{
		if (FAILED(test = this->device->CreatePixelShader(
			XBR5_PS,
			sizeof(XBR5_PS),
			NULL,
			&this->ps)))
		{
		}
	}

	

	
}