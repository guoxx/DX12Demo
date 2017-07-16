#include "Inc/Common.hlsli"
#include "Inc/VSM.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0, visibility=SHADER_VISIBILITY_PIXEL)" \
", DescriptorTable(SRV(t0, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)" \
RootSigEnd

HLSLConstantBuffer(EVSMConstants, 0, g_Constants);

#define SAMPLE_COUNT 4

Texture2DMS<float> g_DepthTexure : register(t0);

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
	uint2 coords = uint2(In.Position.xy);

    float4 average = float4(0.0f, 0.0f, 0.0f, 0.0f);
    for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
	    float d = g_DepthTexure.Load(coords, i);
        average += ConvertToEVSM(d, g_Constants.m_EVSM.m_PositiveExponent, g_Constants.m_EVSM.m_NegativeExponent, SMFormat32Bit);
    }
    return average / SAMPLE_COUNT;
}