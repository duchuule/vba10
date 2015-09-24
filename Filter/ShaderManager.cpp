#include "ShaderManager.h"
#include "../TextureLoader.h"
#include "xBR2_PS.h"
#include "xBR5_PS.h"
#include "hq2x_PS.h"
#include "hq3x_PS.h"
#include "hq4x_PS.h"

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
	const BYTE* shaderByteCode;
	size_t shaderSize;
	this->lutSRV = nullptr;

	if (selection == 2)
	{
		shaderByteCode = HQ2X_PS;
		shaderSize = sizeof(HQ2X_PS);

		//load the Look up table (texture)
		LoadTextureFromFile(
			this->device.Get(),
			L"Filter/hq2x.dds",
			this->lutResource.GetAddressOf(),
			this->lutSRV.GetAddressOf()
			);
	}
	else if (selection == 3)
	{
		shaderByteCode = XBR2_PS;
		shaderSize = sizeof(XBR2_PS);
	}
	else if (selection == 4)
	{
		shaderByteCode = HQ3X_PS;
		shaderSize = sizeof(HQ3X_PS);

		//load the Look up table (texture)
		LoadTextureFromFile(
			this->device.Get(),
			L"Filter/hq3x.dds",
			this->lutResource.GetAddressOf(),
			this->lutSRV.GetAddressOf()
			);
	}
	else if (selection == 5)
	{
		shaderByteCode = XBR5_PS;
		shaderSize = sizeof(XBR5_PS);
	}
	else
	{
		shaderByteCode = HQ4X_PS;
		shaderSize = sizeof(HQ4X_PS);

		//load the Look up table (texture)
		LoadTextureFromFile(
			this->device.Get(),
			L"Filter/hq4x.dds",
			this->lutResource.GetAddressOf(),
			this->lutSRV.GetAddressOf()
			);
	}
	

	if (FAILED(test = this->device->CreatePixelShader(
		shaderByteCode,
		shaderSize,
		NULL,
		&this->ps)))
	{
	}


	
	

	
}