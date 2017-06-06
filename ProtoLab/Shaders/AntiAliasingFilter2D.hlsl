#include "Inc/Common.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", DescriptorTable(SRV(t0, numDescriptors=3), visibility=SHADER_VISIBILITY_PIXEL)" \
RootSigEnd

Texture2D<float4> g_CurrentBuffer : register(t0);
Texture2D<float4> g_HistoryBuffer : register(t1);
Texture2D<float4> g_VelocityBuffer : register(t2);


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

RootSigDeclaration
float4 PSMain(VSOutput In) : SV_TARGET
{
#if 0
	float4 current = g_CurrentBuffer.SampleLevel(g_StaticPointClampSampler, In.Texcoord, 0);
	float4 history = g_HistoryBuffer.SampleLevel(g_StaticPointClampSampler, In.Texcoord, 0);

	return lerp(history, current, 0.05);
#else
	float2 velocity = g_VelocityBuffer.SampleLevel(g_StaticPointClampSampler, In.Texcoord, 0).xy;
	float4 current = g_CurrentBuffer.SampleLevel(g_StaticPointClampSampler, In.Texcoord, 0);
	float4 history = g_HistoryBuffer.SampleLevel(g_StaticPointClampSampler, In.Texcoord + velocity, 0);

	return lerp(history, current, 0.05);
#endif
}