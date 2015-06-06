#ifndef TEXTURELOADER_H_
#define TEXTURELOADER_H_

#include <stdint.h>
#include <D3D11.h>

struct DDS_PIXELFORMAT {
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwFourCC;
  uint32_t dwRGBBitCount;
  uint32_t dwRBitMask;
  uint32_t dwGBitMask;
  uint32_t dwBBitMask;
  uint32_t dwABitMask;
};


typedef struct {
  uint32_t           dwSize;
  uint32_t           dwFlags;
  uint32_t           dwHeight;
  uint32_t           dwWidth;
  uint32_t           dwPitchOrLinearSize;
  uint32_t           dwDepth;
  uint32_t           dwMipMapCount;
  uint32_t           dwReserved1[11];
  DDS_PIXELFORMAT ddspf;
  uint32_t           dwCaps;
  uint32_t           dwCaps2;
  uint32_t           dwCaps3;
  uint32_t           dwCaps4;
  uint32_t           dwReserved2;
} DDS_HEADER;

void LoadTextureFromFile(ID3D11Device1 *device, const wchar_t *filename, ID3D11Resource **texture, ID3D11ShaderResourceView **textureSRV);

#endif