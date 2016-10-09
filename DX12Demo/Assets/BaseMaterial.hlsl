#include "Common.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", CBV(b1, visibility = SHADER_VISIBILITY_ALL)" \
", SRV(t0, visibility = SHADER_VISIBILITY_VERTEX)" \
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


RootSigDeclaration
VSOutput VSMain(uint vertid : SV_VertexID)
{
	VSInput In  = g_VertexArray[vertid];

	VSOutput Out;
	Out.Position = mul(float4(In.Position, 1), g_View.mModelViewProj);

	return Out;
}

RootSigDeclaration
float4 PSMain(VSOutput In) : SV_TARGET
{
	float4 OutColor;
	OutColor = float4(g_Material.Diffuse, 1.0f);
	return OutColor;
}