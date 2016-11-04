#ifndef __RSM_HLSLI__
#define __RSM_HLSLI__

#include "Common.hlsli"

float RSMDistanceFalloffForVPL(float d, float r)
{
	float falloff = 0.0f;
	if (d >= r)
	{
		falloff = 0.0f;
	}
	else
	{
		// linear fall off for simplification
		falloff = (r - d) / r;
	}
	return falloff;
}

float3 ShadeDirectionalLightRSM(GBuffer gbuffer, RSMParam rsmparams, DirectionalLight dirLight,
	Texture2D<float4> RSMRadiantIntensityTex, Texture2D<float4> RSMNormalTex, Texture2D<float> shadowMap, SamplerState samp)
{
	float3 outRadiance = 0.0f;
	if (rsmparams.m_Enabled != 0)
	{
		float3 shadowPos = mul(float4(gbuffer.Position, 1), dirLight.m_mViewProj).xyz;
		float2 shadowUV = float2((shadowPos.x + 1.0f) * 0.5f, (-shadowPos.y + 1.0f) * 0.5f);

		for (uint i = 0; i < g_RSMSamplesCount; ++i)
		{
			float2 uvOffset = g_RSMSamplingPattern[i];
			uvOffset *= rsmparams.m_SampleRadius;

			float2 uv = shadowUV + uvOffset;

			RSMBuffer rsmbuffer = RSMBufferDecode(RSMRadiantIntensityTex, RSMNormalTex, shadowMap, samp, uv, dirLight.m_mInvViewProj, false);

			// for direcional light, irradiance is stored in radiant intensity texture
			float3 VPLIrradiance = rsmbuffer.Intensity;
			// 1/PI already been applied in Diffuse_Lambert, and no distance falloff factor for directional light
			float3 VPLRadiance = VPLIrradiance;
			float3 E = VPLRadiance * PI;

			float3 L = normalize(rsmbuffer.Position - gbuffer.Position);
			float distanceFalloff = RSMDistanceFalloffForVPL(length(rsmbuffer.Position - gbuffer.Position), rsmparams.m_RadiusEnd);
			E = E * saturate(dot(rsmbuffer.Normal, -L)) * saturate(dot(gbuffer.Normal, L)) * distanceFalloff;

			outRadiance += E * Diffuse_Lambert(gbuffer.Diffuse);
		}
		outRadiance /= g_RSMSamplesCount;
		outRadiance *= rsmparams.m_RSMFactor;
	}
	return outRadiance;
}

#endif
