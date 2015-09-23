/*
Hyllian's 5xBR v3.7a (rounded) Shader

Copyright (C) 2011/2012 Hyllian/Jararaca - sergiogdb@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


#include "../SpriteBatch.hlsli"

Texture2D<float4> Texture : register(t0);
sampler TextureSampler : register(s0);
const static float2 texture_size = float2(480.0, 320.0);  //size is the same as size in CreateDeviceDependentResources()


const static float coef = 2.0;
const static float3 yuv_weighted = float3(14.352, 28.176, 5.472);


float4 df(float4 A, float4 B)
{

	// begin optimization: reduction of 42 instruction slots
	return abs(A - B);
	// end optimization

	/* old code

	//return float4(abs(A.x - B.x), abs(A.y - B.y), abs(A.z - B.z), abs(A.w - B.w));
	*/
}


float4 weighted_distance(float4 a, float4 b, float4 c, float4 d, float4 e, float4 f, float4 g, float4 h)
{
	return (df(a, b) + df(a, c) + df(d, e) + df(d, f) + 4.0 * df(g, h));
}



/*    FRAGMENT SHADER    */
float4 main(in PSIn VAR) : SV_TARGET0
{
	bool4 edr, edr_left, edr_up, px; // px = pixel, edr = edge detection rule
	bool4 ir_lv1, ir_lv2_left, ir_lv2_up;
	bool4 nc; // new_color
	bool4 fx, fx_left, fx_up; // inequations of straight lines.

	float2 fp = frac(VAR.texcoord*texture_size);

	float2 ps = float2(1.0 / texture_size.x, 1.0 / texture_size.y);

	float2 dx  = float2(ps.x, 0); // F
	float2 dy = float2(0, ps.y); // H

	float3 A = Texture.Sample(TextureSampler, VAR.texcoord - dx - dy).xyz;
	float3 B = Texture.Sample(TextureSampler, VAR.texcoord - dy).xyz;
	float3 C = Texture.Sample(TextureSampler, VAR.texcoord + dx - dy).xyz;
	float3 D = Texture.Sample(TextureSampler, VAR.texcoord - dx).xyz;
	float3 E = Texture.Sample(TextureSampler, VAR.texcoord).xyz;
	float3 F = Texture.Sample(TextureSampler, VAR.texcoord + dx).xyz;
	float3 G = Texture.Sample(TextureSampler, VAR.texcoord - dx + dy).xyz;
	float3 H = Texture.Sample(TextureSampler, VAR.texcoord + dy).xyz;
	float3 I = Texture.Sample(TextureSampler, VAR.texcoord + dx + dy).xyz;

	float3  A1 = Texture.Sample(TextureSampler, VAR.texcoord - dx - 2.0*dy).xyz;
	float3  C1 = Texture.Sample(TextureSampler, VAR.texcoord + dx - 2.0*dy).xyz;
	float3  A0 = Texture.Sample(TextureSampler, VAR.texcoord - 2.0*dx - dy).xyz;
	float3  G0 = Texture.Sample(TextureSampler, VAR.texcoord - 2.0*dx + dy).xyz;
	float3  C4 = Texture.Sample(TextureSampler, VAR.texcoord + 2.0*dx - dy).xyz;
	float3  I4 = Texture.Sample(TextureSampler, VAR.texcoord + 2.0*dx + dy).xyz;
	float3  G5 = Texture.Sample(TextureSampler, VAR.texcoord - dx + 2.0*dy).xyz;
	float3  I5 = Texture.Sample(TextureSampler, VAR.texcoord + dx + 2.0*dy).xyz;
	float3  B1 = Texture.Sample(TextureSampler, VAR.texcoord - 2.0*dy).xyz;
	float3  D0 = Texture.Sample(TextureSampler, VAR.texcoord - 2.0*dx).xyz;
	float3  H5 = Texture.Sample(TextureSampler, VAR.texcoord + 2.0*dy).xyz;
	float3  F4 = Texture.Sample(TextureSampler, VAR.texcoord + 2.0*dx).xyz;

	float4 b = mul(float4x3(B, D, H, F), yuv_weighted);
	float4 c = mul(float4x3(C, A, G, I), yuv_weighted);
	float4 e = mul(float4x3(E, E, E, E), yuv_weighted);
	float4 d = b.yzwx;
	float4 f = b.wxyz;
	float4 g = c.zwxy;
	float4 h = b.zwxy;
	float4 i = c.wxyz;

	float4 i4 = mul(float4x3(I4, C1, A0, G5), yuv_weighted);
	float4 i5 = mul(float4x3(I5, C4, A1, G0), yuv_weighted);
	float4 h5 = mul(float4x3(H5, F4, B1, D0), yuv_weighted);
	float4 f4 = h5.yzwx;

	float4 Ao = float4(1.0, -1.0, -1.0, 1.0);
	float4 Bo = float4(1.0,  1.0, -1.0,-1.0);
	float4 Co = float4(1.5,  0.5, -0.5, 0.5);
	float4 Ax = float4(1.0, -1.0, -1.0, 1.0);
	float4 Bx = float4(0.5,  2.0, -0.5,-2.0);
	float4 Cx = float4(1.0,  1.0, -0.5, 0.0);
	float4 Ay = float4(1.0, -1.0, -1.0, 1.0);
	float4 By = float4(2.0,  0.5, -2.0,-0.5);
	float4 Cy = float4(2.0,  0.0, -1.0, 0.5);

	// These inequations define the line below which interpolation occurs.
	fx.x = (Ao.x*fp.y + Bo.x*fp.x > Co.x);
	fx_left.x = (Ax.x*fp.y + Bx.x*fp.x > Cx.x);
	fx_up.x = (Ay.x*fp.y + By.x*fp.x > Cy.x);

	fx.y = (Ao.y*fp.y + Bo.y*fp.x > Co.y);
	fx_left.y = (Ax.y*fp.y + Bx.y*fp.x > Cx.y);
	fx_up.y = (Ay.y*fp.y + By.y*fp.x > Cy.y);

	fx.z = (Ao.z*fp.y + Bo.z*fp.x > Co.z);
	fx_left.z = (Ax.z*fp.y + Bx.z*fp.x > Cx.z);
	fx_up.z = (Ay.z*fp.y + By.z*fp.x > Cy.z);

	fx.w = (Ao.w*fp.y + Bo.w*fp.x > Co.w);
	fx_left.w = (Ax.w*fp.y + Bx.w*fp.x > Cx.w);
	fx_up.w = (Ay.w*fp.y + By.w*fp.x > Cy.w);

	//ir_lv1.x = ((e.x != f.x) && (e.x != h.x));
	//ir_lv1.y = ((e.y != f.y) && (e.y != h.y));
	//ir_lv1.z = ((e.z != f.z) && (e.z != h.z));
	//ir_lv1.w = ((e.w != f.w) && (e.w != h.w));
	ir_lv1 = ((e != f) && (e != h));

	//ir_lv2_left.x = ((e.x != g.x) && (d.x != g.x));
	//ir_lv2_left.y = ((e.y != g.y) && (d.y != g.y));
	//ir_lv2_left.z = ((e.z != g.z) && (d.z != g.z));
	//ir_lv2_left.w = ((e.w != g.w) && (d.w != g.w));
	ir_lv2_left = ((e != g) && (d != g));

	//ir_lv2_up.x = ((e.x != c.x) && (b.x != c.x));
	//ir_lv2_up.y = ((e.y != c.y) && (b.y != c.y));
	//ir_lv2_up.z = ((e.z != c.z) && (b.z != c.z));
	//ir_lv2_up.w = ((e.w != c.w) && (b.w != c.w));
	ir_lv2_up = ((e != c) && (b != c));

	float4 w1 = weighted_distance(e, c, g, i, h5, f4, h, f);
	float4 w2 = weighted_distance(h, d, i5, f, i4, b, e, i);

	float4 t1 = (coef*df(f,g));
	float4 t2 = df(h,c);
	float4 t3 = df(f,g);
	float4 t4 = (coef*df(h,c));

	//edr = bool4((w1.x < w2.x) && ir_lv1.x, 
	//            (w1.y < w2.y) && ir_lv1.y, 
	//            (w1.z < w2.z) && ir_lv1.z, 
	//            (w1.w < w2.w) && ir_lv1.w);
	edr = (w1 < w2) && ir_lv1;

	//edr_left = bool4((t1.x <= t2.x) && ir_lv2_left.x, 
	//                 (t1.y <= t2.y) && ir_lv2_left.y, 
	//                 (t1.z <= t2.z) && ir_lv2_left.z, 
	//                 (t1.w <= t2.w) && ir_lv2_left.w);
	edr_left = (t1 <= t2) && ir_lv2_left;

	//edr_up = bool4((t4.x <= t3.x) && ir_lv2_up.x, 
	//               (t4.y <= t3.y) && ir_lv2_up.y, 
	//               (t4.z <= t3.z) && ir_lv2_up.z, 
	//               (t4.w <= t3.w) && ir_lv2_up.w);
	edr_up = (t4 <= t3) && ir_lv2_up;

	//nc.x = (edr.x && (fx.x || edr_left.x && fx_left.x || edr_up.x && fx_up.x));
	//nc.y = (edr.y && (fx.y || edr_left.y && fx_left.y || edr_up.y && fx_up.y));
	//nc.z = (edr.z && (fx.z || edr_left.z && fx_left.z || edr_up.z && fx_up.z));
	//nc.w = (edr.w && (fx.w || edr_left.w && fx_left.w || edr_up.w && fx_up.w));
	nc = (edr && (fx || edr_left && fx_left || edr_up && fx_up));


	t1 = df(e,f);
	t2 = df(e,h);

	//px = bool4(t1.x <= t2.x, 
	//           t1.y <= t2.y, 
	//           t1.z <= t2.z, 
	//           t1.w <= t2.w);
	px = t1 <= t2;

	float3 res = nc.x ? px.x ? F : H : nc.y ? px.y ? B : F : nc.z ? px.z ? D : B : nc.w ? px.w ? H : D : E;

	return float4(res.x, res.y, res.z, 1.0);
}