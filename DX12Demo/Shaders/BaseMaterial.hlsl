#include "Inc/Common.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", SRV(t0, visibility = SHADER_VISIBILITY_VERTEX)" \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", CBV(b1, visibility = SHADER_VISIBILITY_ALL)" \
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

struct View
{
	float4x4 mModelViewProj;
	float4x4 mInverseTransposeModel;
};

struct BaseMaterial
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Transmittance;
	float4 Emission;
	float4 Shininess;
	float4 Ior;
	float4 Dissolve;
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
	Out.Normal = mul(float4(In.Normal, 0), g_View.mInverseTransposeModel).xyz;
	Out.Texcoord = In.Texcoord;

	return Out;
}

RootSigDeclaration
GBufferOutput PSMain(VSOutput In)
{
	GBuffer gbuffer;

	gbuffer.Diffuse = g_DiffuseTexture.Sample(s_PointSampler, In.Texcoord).xyz;
	gbuffer.Specular = IorToF0_Dielectric(g_Material.Ior.x).xxx;
	gbuffer.Normal = In.Normal;
	gbuffer.Roughness = saturate((100.0f - g_Material.Shininess) / 100.0f);

	return GBufferEncode(gbuffer);
}