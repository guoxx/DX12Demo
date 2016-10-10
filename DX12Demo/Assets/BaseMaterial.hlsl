#include "Common.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", CBV(b1, visibility = SHADER_VISIBILITY_ALL)" \
", SRV(t0, visibility = SHADER_VISIBILITY_VERTEX)" \
", DescriptorTable(SRV(t1, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)" \
", StaticSampler(s0, filter=FILTER_MIN_MAG_MIP_POINT, visibility=SHADER_VISIBILITY_PIXEL)" \
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
	float2 Texcoord : TEXCOORD;
};

struct View
{
	float4x4 mModelViewProj;
};

struct BaseMaterial
{
	float3 Ambient;
	float3 Diffuse;
	float3 Specular;
	float3 Transmittance;
	float3 Emission;
	float Shininess;
	float Ior;
	float Dissolve;
};

ConstantBuffer<View> g_View : register(b0);
ConstantBuffer<BaseMaterial> g_Material : register(b1);
StructuredBuffer<VSInput> g_VertexArray : register(t0);
Texture2D<float4> g_DiffuseTexture : register(t1);
SamplerState s_PointSampler : register(s0);


RootSigDeclaration
VSOutput VSMain(uint vertid : SV_VertexID)
{
	VSInput In  = g_VertexArray[vertid];

	VSOutput Out;
	Out.Position = mul(float4(In.Position, 1), g_View.mModelViewProj);
	Out.Texcoord = In.Texcoord;

	return Out;
}

RootSigDeclaration
float4 PSMain(VSOutput In) : SV_TARGET
{
	float4 OutColor = g_DiffuseTexture.Sample(s_PointSampler, In.Texcoord);
	//OutColor = float4(g_Material.Diffuse + g_Material.Specular + g_Material.Ambient, 1.0f);
	return OutColor;
}