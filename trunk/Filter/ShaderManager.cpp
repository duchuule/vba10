#include "ShaderManager.h"
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
	if (FAILED(test = this->device->CreatePixelShader(
		XBR5_PS,
		sizeof(XBR5_PS),
		NULL,
		&this->ps)))
	{

	}

	
}