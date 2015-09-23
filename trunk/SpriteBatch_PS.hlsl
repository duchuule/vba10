#include "SpriteBatch.hlsli"

Texture2D<float4> Texture : register(t0);
sampler TextureSampler : register(s0);

float4 SpriteBatch_PS(PSIn input) : SV_TARGET0
{
	//if color is white (1, 1, 1), multiply by input.color does nothing
	//but we could use this to create color filter
	return Texture.Sample(TextureSampler, input.texcoord) *input.color;  
}