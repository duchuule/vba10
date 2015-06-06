#ifndef ONRESIZEBUFFER_H_
#define ONRESIZEBUFFER_H_

#include <D3D11.h>
#include <directxmath.h>

using namespace DirectX;

namespace Engine
{
	struct OnResizeBuffer
	{
		XMMATRIX projection;
	};
}

#endif