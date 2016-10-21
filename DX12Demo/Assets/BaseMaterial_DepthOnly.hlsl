#include "Common.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", SRV(t0, visibility = SHADER_VISIBILITY_VERTEX)" \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
RootSigEnd


struct VSInput
{
	float3 Position;
	float3 Normal;
	float2 Texcoord;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
};

struct View
{
	float4x4 mModelViewProj;
};

ConstantBuffer<View> g_View : register(b0);
StructuredBuffer<VSInput> g_VertexArray : register(t0);


RootSigDeclaration
VSOutput VSMain(uint vertid : SV_VertexID)
{
	VSInput In  = g_VertexArray[vertid];

	VSOutput Out;
	Out.Position = mul(float4(In.Position, 1), g_View.mModelViewProj);

	return Out;
}
