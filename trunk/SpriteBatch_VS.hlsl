#include "SpriteBatch.hlsli"

cbuffer OnResize : register(b0)
{
	row_major float4x4 Projection;
}

cbuffer OnResize : register(b1)
{
	row_major float4x4 World;
}


struct VSIn
{
	float4 position				:	POSITION;
	float4 color				:	COLOR0;
	float2 texcoord				:	TEXCOORD0;
};

PSIn SpriteBatch_VS(VSIn input)
{
	PSIn result = (PSIn)0;
	result.color = input.color;
	result.position = mul(input.position, mul(World, Projection));
	result.texcoord = input.texcoord;

	return result;
}