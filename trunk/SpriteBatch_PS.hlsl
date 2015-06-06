#include "SpriteBatch.hlsli"

Texture2D<float4> Texture : register(t0);
sampler TextureSampler : register(s0);

float4 SpriteBatch_PS(PSIn input) : SV_TARGET0
{
	return Texture.Sample(TextureSampler, input.texcoord) * input.color;
}