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

	int LightType;
	int Padding0;
	int Padding1;
	int Padding2;

	float4 DirectionalLightIrradiance;
	float4 DirectionalLightDirection;
	float4 PointLightIntensity;
	float4 PointLightPosition;
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

	float3 diffuse = g_DiffuseTexture.Sample(s_PointSampler, In.Texcoord).xyz;
	float3 reflectedDiffuse = Diffuse_Lambert(diffuse);
	float3 I = 0;
	if (HLSL_CB_GET(0, LightType) == 0)
	{
		// directional light
		float3 L = -HLSL_CB_GET(0, DirectionalLightDirection).xyz;
		float3 N = normalize(In.Normal);
		float NdotL = saturate(dot(N, L));
		float3 E = HLSL_CB_GET(0, DirectionalLightIrradiance).xyz;
		// store irradiance as intensity, because you are not be able to evalute the intensity for a directional light
		// I = E * (d^2), d^2 will be factored out in further calculation
		I = E * NdotL * reflectedDiffuse;
	}
	else
	{
		// point light
		float3 L = normalize(HLSL_CB_GET(0, PointLightPosition).xyz - In.Position.xyz);
		float3 N = normalize(In.Normal);
		float NdotL = saturate(dot(L, N));
		I = HLSL_CB_GET(0, PointLightIntensity).xyz * NdotL * reflectedDiffuse;
	}

	rsmbuffer.Intensity = I;
	rsmbuffer.Normal = In.Normal;

	return RSMBufferEncode(rsmbuffer);
}