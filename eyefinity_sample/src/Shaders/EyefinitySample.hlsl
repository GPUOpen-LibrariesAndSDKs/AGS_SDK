//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

//--------------------------------------------------------------------------------------
// File: EyefinitySample.hlsl
//--------------------------------------------------------------------------------------

struct VSSceneIn
{
	float3 pos	: POSITION;			//position
	float3 norm : NORMAL;			//normal
	float2 tex	: TEXTURE0;			//texture coordinate
};

struct PSSceneIn
{
	float4 pos : SV_Position;
	float2 tex : TEXTURE0;
};

// Textures
Texture2D g_txDiffuse		: register( t0 );

// Samplers
SamplerState g_SampleLinear : register( s0 );

// Comstant
cbuffer cb0 : register( b0 )
{
	float4x4 g_mWorldViewProj;
};

PSSceneIn VSScenemain(VSSceneIn input)
{
	PSSceneIn output;
	
	output.pos = mul( float4(input.pos,1.0), g_mWorldViewProj );
	output.tex = input.tex;
	
	return output;
}

float4 PSScenemain(PSSceneIn input) : SV_Target
{	
	return g_txDiffuse.Sample( g_SampleLinear, input.tex );
}