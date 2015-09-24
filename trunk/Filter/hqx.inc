/*
* Copyright (C) 2003 Maxim Stepin ( maxst@hiend3d.com )
*
* Copyright (C) 2010 Cameron Zemek ( grom@zeminvaders.net )
*
* Copyright (C) 2014 Jules Blok ( jules@aerix.nl )
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

//code from https://github.com/Armada651/hqx-shader/blob/master/single-pass/shader-files/hqx.inc


#include "../SpriteBatch.hlsli"

Texture2D<float4> Texture : register(t0);
Texture2D<float4> LUT : register(t1);
sampler TextureSampler : register(s0);
const static float2 texture_size = float2(480.0, 320.0);  //size is the same as size in CreateDeviceDependentResources()


#ifdef PARAMETER_UNIFORM
uniform float trY;
uniform float trU;
uniform float trV;
#else
#define trY 48.0
#define trU 7.0
#define trV 6.0
#endif

static float3 yuv_threshold = float3(trY/255.0, trU/255.0, trV/255.0);

const static float3x3 yuv = float3x3(0.299, 0.587, 0.114, -0.169, -0.331, 0.5, 0.5, -0.419, -0.081);
const static float3 yuv_offset = float3(0, 0.5, 0.5);

bool diff(float3 yuv1, float3 yuv2) {
	bool3 res = abs((yuv1 + yuv_offset) - (yuv2 + yuv_offset)) > yuv_threshold;
	return res.x || res.y || res.z;
}






/*    FRAGMENT SHADER    */
float4 main(in PSIn VAR) : SV_TARGET0
{

	//<------ move from vertex shader in original code

	float2 ps = 1.0/texture_size;
	float dx = ps.x;
	float dy = ps.y;


	//   +----+----+----+
	//   |    |    |    |
	//   | w1 | w2 | w3 |
	//   +----+----+----+
	//   |    |    |    |
	//   | w4 | w5 | w6 |
	//   +----+----+----+
	//   |    |    |    |
	//   | w7 | w8 | w9 |
	//   +----+----+----+

	// This line fix a bug in ATI cards.
	float2 texCoord = VAR.texcoord + float2(0.0000001, 0.0000001);

	float4 t1 = texCoord.xxxy + float4(-dx, 0, dx, -dy); //  w1 | w2 | w3
	float4 t2 = texCoord.xxxy + float4(-dx, 0, dx,   0); //  w4 | w5 | w6
	float4 t3 = texCoord.xxxy + float4(-dx, 0, dx,  dy); //  w7 | w8 | w9

	//------------->


	float2 fp = frac(texCoord*texture_size);
	float2 quad = sign(-0.5 + fp);


	float3 p1 = Texture.Sample(TextureSampler, texCoord).rgb;
	float3 p2 = Texture.Sample(TextureSampler, texCoord + float2(dx, dy) * quad).rgb;
	float3 p3 = Texture.Sample(TextureSampler, texCoord + float2(dx, 0) * quad).rgb;
	float3 p4 = Texture.Sample(TextureSampler, texCoord + float2(0, dy) * quad).rgb;
	float4x3 pixels = float4x3(p1, p2, p3, p4);

	float3 w1  = mul(yuv, Texture.Sample(TextureSampler, t1.xw).rgb);
	float3 w2  = mul(yuv, Texture.Sample(TextureSampler, t1.yw).rgb);
	float3 w3  = mul(yuv, Texture.Sample(TextureSampler, t1.zw).rgb);

	float3 w4  = mul(yuv, Texture.Sample(TextureSampler, t2.xw).rgb);
	float3 w5  = mul(yuv, p1);
	float3 w6  = mul(yuv, Texture.Sample(TextureSampler, t2.zw).rgb);

	float3 w7  = mul(yuv, Texture.Sample(TextureSampler, t3.xw).rgb);
	float3 w8  = mul(yuv, Texture.Sample(TextureSampler, t3.yw).rgb);
	float3 w9  = mul(yuv, Texture.Sample(TextureSampler, t3.zw).rgb);

	bool3x3 pattern = bool3x3(diff(w5, w1), diff(w5, w2), diff(w5, w3),
	                          diff(w5, w4), false       , diff(w5, w6),
	                          diff(w5, w7), diff(w5, w8), diff(w5, w9));
	bool4 cross = bool4(diff(w4, w2), diff(w2, w6), diff(w8, w4), diff(w6, w8));
	
	float2 index;
	index.x = dot(pattern[0], float3(1, 2, 4)) +
	          dot(pattern[1], float3(8, 0, 16)) +
	          dot(pattern[2], float3(32, 64, 128));
	index.y = dot(cross, float4(1, 2, 4, 8)) * (SCALE * SCALE) +
              dot(floor(fp * SCALE), float2(1, SCALE));

	float2 step = 1.0 / float2(256.0, 16.0 * (SCALE * SCALE));
	float2 offset = step / 2.0;
	float4 weights = LUT.Sample(TextureSampler, index * step + offset);
	float sum = dot(weights, float4(1, 1, 1, 1));
	float3 res = mul(transpose(pixels), weights / sum);

	return float4(res, 1.0);





}