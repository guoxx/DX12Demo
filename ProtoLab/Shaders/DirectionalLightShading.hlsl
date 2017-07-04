#include "Inc/Common.hlsli"
#include "Inc/RSM.hlsli"
#include "Inc/VSM.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", DescriptorTable(SRV(t0, numDescriptors=9), visibility=SHADER_VISIBILITY_PIXEL)" \
RootSigEnd

HLSLConstantBuffer(DirectionalLightConstants, 0, g_Constants);

Texture2D<float4> g_GBuffer0 : register(t0);
Texture2D<float4> g_GBuffer1 : register(t1);
Texture2D<float4> g_GBuffer2 : register(t2);
Texture2D<float4> g_GBuffer3 : register(t3);
Texture2D<float> g_DepthTexture : register(t4);
Texture2D<float> g_ShadowMap : register(t5);
Texture2D<float4> g_RSMIntensityTexture : register(t6);
Texture2D<float4> g_RSMNormalTexture : register(t7);
Texture2D<float4> g_EVSMTexture : register(t8);


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

float ShadowMask(GBuffer gbuffer)
{
    // TODO: don't hardcore the size
    float2 shadowMapsSize = float2(2048, 2048);

	float3 shadowPos;
	float2 shadowUV;

    int cascadeIdx = 3;
    for (int i = 3; i >=0; --i)
    {
	    float3 pos = mul(float4(gbuffer.Position, 1), g_Constants.m_DirLight.m_mViewProj[i]).xyz;
	    float2 uv = float2((pos.x + 1.0f) * 0.5f, (-pos.y + 1.0f) * 0.5f);
        float border = 4;
        if (all(abs(pos.xy) < ((shadowMapsSize / 4 - border) / (shadowMapsSize / 4))))
        {
            cascadeIdx = i;
            shadowPos = pos;

            int x = cascadeIdx & 0x01;
            int y = (cascadeIdx & 0x02) > 1;

            shadowUV.x = x * 0.5 + uv.x * 0.5;
            shadowUV.y = y * 0.5 + uv.y * 0.5;
        }
    }

	float shadowMask = 1.0f;
	if (g_Constants.m_EVSM.m_Enabled)
	{
		float2 exponents = GetEVSMExponents(g_Constants.m_EVSM.m_PositiveExponent, g_Constants.m_EVSM.m_NegativeExponent, SMFormat32Bit);
		float2 warpedDepth = WarpDepth(shadowPos.z, exponents);

		float4 occluder = g_EVSMTexture.Sample(g_StaticAnisoClampSampler, shadowUV);

		// Derivative of warping at depth
		float2 depthScale = g_Constants.m_EVSM.m_VSMBias * 0.01f * exponents * warpedDepth;
		float2 minVariance = depthScale * depthScale;

		float posContrib = ChebyshevUpperBound(occluder.xz, warpedDepth.x, minVariance.x, g_Constants.m_EVSM.m_LightBleedingReduction);
		float negContrib = ChebyshevUpperBound(occluder.yw, warpedDepth.y, minVariance.y, g_Constants.m_EVSM.m_LightBleedingReduction);
		shadowMask = min(posContrib, negContrib);
	}
	else
	{
		float occluderDepth = g_ShadowMap.Sample(g_StaticPointClampSampler, shadowUV);
		shadowMask = occluderDepth < shadowPos.z ? 0.0f : 1.0f;
	}
	return shadowMask;
}

RootSigDeclaration
float4 PSMain(VSOutput In) : SV_TARGET
{
	GBuffer gbuffer = GBufferDecode(g_GBuffer0, g_GBuffer1, g_GBuffer2, g_GBuffer3, g_DepthTexture, g_StaticPointClampSampler, In.Texcoord, g_Constants.mInvView, g_Constants.mInvProj);

	float3 outRadiance = 0.0f;

	float shadowMask = ShadowMask(gbuffer);

	float3 L = -g_Constants.m_DirLight.m_Direction;
	float3 V = normalize(g_Constants.CameraPosition.xyz - gbuffer.Position);
	float3 N = gbuffer.Normal;
	float NdotL = saturate(dot(N, L));
	float3 E = g_Constants.m_DirLight.m_Irradiance;

	float3 diffuse = Diffuse_Lambert(gbuffer.Diffuse);
	float3 specular = MicrofacetSpecular(gbuffer.Specular, gbuffer.Roughness, V, N, L);
	outRadiance += (diffuse + specular) * shadowMask * E * NdotL;

	if (g_Constants.m_RSM.m_Enabled)
	{
		outRadiance += ShadeDirectionalLightRSM(gbuffer, g_Constants.m_RSM, g_Constants.m_DirLight,
			g_RSMIntensityTexture, g_RSMNormalTexture, g_ShadowMap, g_StaticPointClampSampler);
	}

    // TODO - ambient
    outRadiance += gbuffer.Diffuse * 0.1;

	return float4(outRadiance, 1);
}