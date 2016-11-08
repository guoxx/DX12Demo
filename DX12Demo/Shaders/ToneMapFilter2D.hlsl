#include "Inc/Common.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", RootConstants(num32BitConstants=1, b0)" \
", DescriptorTable(SRV(t0, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)" \
RootSigEnd

Texture2D<float4> g_Texture : register(t0);


cbuffer cb0 : register(b0)
{
	float Exposure;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD;
};

RootSigDeclaration
VSOutput VSMain(uint vertid : SV_VertexID)
{
	// 0 (0, 0)
	// 1 (1, 0)
	// 2 (0, 1)
	// 3 (1, 1)

	int x = vertid & 0x01;
	int y = (vertid >> 1) & 0x01;

	VSOutput Out;
	Out.Position = float4(x * 2 - 1, y * 2 - 1, 0, 1);
	Out.Texcoord = float2(x, 1 - y);

	return Out;
}

float3 ToneMap( float3 hdr, float E = 4.0 )
{
	return hdr * pow(2, E);
}

RootSigDeclaration
float4 PSMain(VSOutput In) : SV_TARGET
{
	float4 HdrColor = g_Texture.Sample(g_StaticPointClampSampler, In.Texcoord);

	float3 SdrColor = ToneMap( HdrColor.xyz, Exposure );

	return float4(SdrColor, saturate(HdrColor.a));
}