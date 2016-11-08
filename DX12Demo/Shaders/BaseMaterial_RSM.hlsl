#include "Inc/Common.hlsli"
#include "Inc/RSM.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", SRV(t0, visibility = SHADER_VISIBILITY_VERTEX)" \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", DescriptorTable(SRV(t1, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)" \
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

HLSLConstantBuffer(BaseMaterialRSMConstants, 0, g_Constants)

StructuredBuffer<VSInput> g_VertexArray : register(t0);
Texture2D<float4> g_DiffuseTexture : register(t1);


RootSigDeclaration
VSOutput VSMain(uint vertid : SV_VertexID)
{
	VSInput In  = g_VertexArray[vertid];

	VSOutput Out;
	Out.Position = mul(float4(In.Position, 1), g_Constants.mModelViewProj);
	Out.Normal = mul(float4(In.Normal, 0), g_Constants.mInverseTransposeModel).xyz;
	Out.Texcoord = In.Texcoord;

	return Out;
}

RootSigDeclaration
RSMOutput PSMain(VSOutput In)
{
	RSMBuffer rsmbuffer;

	float3 diffuse = g_DiffuseTexture.Sample(g_StaticAnisoWrapSampler, In.Texcoord).xyz;
	float3 reflectedDiffuse = Diffuse_Lambert(diffuse);
	float3 I = 0;
	if (g_Constants.LightType == 0)
	{
		// directional light
		float3 L = -g_Constants.DirectionalLightDirection.xyz;
		float3 N = normalize(In.Normal);
		float NdotL = saturate(dot(N, L));
		float3 E = g_Constants.DirectionalLightIrradiance.xyz;
		// store irradiance as intensity, because you are not be able to evalute the intensity for a directional light
		// I = E * (d^2), d^2 will be factored out in further calculation
		I = E * PI * NdotL * reflectedDiffuse;
	}
	else
	{
		// point light
		float3 L = normalize(g_Constants.PointLightPosition.xyz - In.Position.xyz);
		float3 N = normalize(In.Normal);
		float NdotL = saturate(dot(L, N));
		I = g_Constants.PointLightIntensity.xyz * PI * NdotL * reflectedDiffuse;
	}

	rsmbuffer.Intensity = I;
	rsmbuffer.Normal = In.Normal;

	return RSMBufferEncode(rsmbuffer);
}