#include "ShaderManager.h"
#include "xBR2_PS.h"
#include "xBR5_PS.h"

using namespace VBA10;
using namespace Microsoft::WRL;

ShaderManager *ShaderManager::instance = nullptr;

ShaderManager::ShaderManager(ID3D11Device1 *device):device(device)
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
	if (selection == 1)
	{
		shaderByteCode = XBR2_PS;
		shaderSize = sizeof(XBR2_PS);
	}
	else
	{
		shaderByteCode = XBR5_PS;
		shaderSize = sizeof(XBR5_PS);
	}


	if (FAILED(test = this->device->CreatePixelShader(
		shaderByteCode,
		shaderSize,
		NULL,
		&this->ps)))
	{
		int test = sizeof(*shaderByteCode);
		test++;
	}

	
}