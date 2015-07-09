#include "pch.h"
#include "TextureLoader.h"

using namespace Microsoft::WRL;

void LoadTextureFromFile(ID3D11Device1 *device, const wchar_t *filename, ID3D11Resource **texture, ID3D11ShaderResourceView **textureSRV)
{
	FILE *file;
	if(_wfopen_s(&file, filename, L"rb") != 0)
	{
		return;
	}

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	if(fileSize < sizeof(DDS_HEADER) + sizeof(uint32_t))
	{
		return;
	}

	char *bytes = new char[fileSize];
	if(fread(bytes, 1, fileSize, file) != fileSize)
	{
		fclose(file);
		delete [] bytes;
		return;
	}
	fclose(file);

	const char *magic = "DDS ";
	uint32_t magicValue = *(reinterpret_cast<const uint32_t *>(magic));

	char *magicFile = bytes;
	uint32_t magicFileValue = *(const uint32_t *) magicFile;

	if(magicFileValue != magicValue)
	{
		delete [] bytes;
		return;
	}

	DDS_HEADER *header = reinterpret_cast<DDS_HEADER *>(bytes + sizeof(uint32_t));

	int width = header->dwWidth;
	int height = header->dwHeight;


	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.Height = height;
	desc.Width = width;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ArraySize = 1;
	desc.MipLevels = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

	D3D11_SUBRESOURCE_DATA pixelData;
	ZeroMemory(&pixelData, sizeof(D3D11_SUBRESOURCE_DATA));
	pixelData.pSysMem = bytes + sizeof(DDS_HEADER) + sizeof(uint32_t);
	pixelData.SysMemPitch = 4 * width;
	
	ID3D11Texture2D *tex;
	if(FAILED(device->CreateTexture2D(&desc, &pixelData, &tex)))
	{
		delete [] bytes;
		return;
	}

	delete [] bytes;


	D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
	ZeroMemory(&descSRV, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

    descSRV.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	descSRV.Texture2D.MipLevels = 1;

	if(FAILED(device->CreateShaderResourceView(tex, &descSRV, textureSRV)))
	{
		tex->Release();
		return;
	}
	*texture = tex;
}
