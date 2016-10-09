#include "Common.hlsli"

#define RootSigDeclaration \
RootSigStart \
", SRV(t0, visibility = SHADER_VISIBILITY_VERTEX)"
", CBV(b1, visibility = SHADER_VISIBILITY_ALL)"
RootSigEnd


struct VSInput
{
	float3 Position;
	float3 Normal;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
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
	int Illum;
};

StructuredBuffer<VSInput> g_VertexArray : register(t0);

ConstantBuffer<BaseMaterial> g_Material : register(b1)


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