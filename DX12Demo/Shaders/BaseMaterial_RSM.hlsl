#include "Inc/Common.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", SRV(t0, visibility = SHADER_VISIBILITY_VERTEX)" \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", DescriptorTable(SRV(t1, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)" \
", StaticSampler(s0, filter=FILTER_ANISOTROPIC, visibility=SHADER_VISIBILITY_PIXEL)" \
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
	float3 Normal : NORMAL;
	float2 Texcoord : TEXCOORD;
};

HLSL_CB_DECL(BaseMaterial_RSM, Constants, 0,
{
	float4x4 mModelViewProj;
	float4x4 mInverseTransposeModel;
});

StructuredBuffer<VSInput> g_VertexArray : register(t0);
Texture2D<float4> g_DiffuseTexture : register(t1);
SamplerState s_PointSampler : register(s0);


RootSigDeclaration
VSOutput VSMain(uint vertid : SV_VertexID)
{
	VSInput In  = g_VertexArray[vertid];

	VSOutput Out;
	Out.Position = mul(float4(In.Position, 1), HLSL_CB_GET(0, mModelViewProj));
	Out.Normal = mul(float4(In.Normal, 0), HLSL_CB_GET(0, mInverseTransposeModel)).xyz;
	Out.Texcoord = In.Texcoord;

	return Out;
}

RootSigDeclaration
RSMOutput PSMain(VSOutput In)
{
	RSMBuffer rsmbuffer;

	rsmbuffer.Intensity = g_DiffuseTexture.Sample(s_PointSampler, In.Texcoord).xyz;
	rsmbuffer.Normal = In.Normal;

	return RSMBufferEncode(rsmbuffer);
}