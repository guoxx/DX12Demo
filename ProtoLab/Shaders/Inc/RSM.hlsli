#ifndef __RSM_HLSLI__
#define __RSM_HLSLI__

#include "Common.hlsli"
#include "GBuffer.hlsli"
#include "Utils.hlsli"
#include "BRDF.hlsli"

struct RSMOutput
{
	float4 Intensity	: SV_TARGET0;
	float4 Normal		: SV_TARGET1;
};

struct RSMBuffer
{
	float3 Intensity;
	float3 Normal;

	// --
	float Depth;
	float3 Position;
};

RSMOutput RSMBufferEncode(RSMBuffer rsmbuffer)
{
	RSMOutput Out;
	Out.Intensity = float4(rsmbuffer.Intensity, 1.0f);
	Out.Normal = float4((rsmbuffer.Normal + 1.0f) * 0.5f, 1.0f);
	return Out;
}

RSMBuffer RSMBufferDecode(Texture2D<float4> RT0, Texture2D<float4> RT1, Texture2D<float> DepthBuffer, SamplerState samp, float2 UV, float2 normalizedUV, float4x4 mInvViewProj, bool perspectiveDivide)
{
	RSMBuffer rsmbuffer;

	float4 Intensity = RT0.SampleLevel(samp, UV, 0);
	float4 Normal= RT1.SampleLevel(samp, UV, 0);
	rsmbuffer.Intensity = Intensity.xyz;
	rsmbuffer.Normal = Normal.xyz * 2.0f - 1.0f;

	rsmbuffer.Depth = DepthBuffer.SampleLevel(samp, UV, 0);

	// Assume DX style texture coordinate
	float3 ndcPos = float3(normalizedUV.x * 2.0f - 1.0f, (1.0f - normalizedUV.y) * 2.0f - 1.0f, rsmbuffer.Depth);
	float4 wsPos = mul(float4(ndcPos, 1), mInvViewProj);
	if (perspectiveDivide)
	{
		wsPos /= wsPos.w;
	}
	rsmbuffer.Position = wsPos.xyz;

	return rsmbuffer;
}

RSMBuffer RSMBufferDecode(Texture2D<float4> RT0, Texture2D<float4> RT1, Texture2D<float4> DepthBuffer, SamplerState samp, float2 UV, float2 normalizedUV, float4x4 mInvViewProj, bool perspectiveDivide)
{
	RSMBuffer rsmbuffer;

	float4 Intensity = RT0.SampleLevel(samp, UV, 0);
	float4 Normal= RT1.SampleLevel(samp, UV, 0);
	rsmbuffer.Intensity = Intensity.xyz;
	rsmbuffer.Normal = Normal.xyz * 2.0f - 1.0f;

	rsmbuffer.Depth = DepthBuffer.SampleLevel(samp, UV, 0).x;

	// Assume DX style texture coordinate
	float3 ndcPos = float3(normalizedUV.x * 2.0f - 1.0f, (1.0f - normalizedUV.y) * 2.0f - 1.0f, rsmbuffer.Depth);
	float4 wsPos = mul(float4(ndcPos, 1), mInvViewProj);
	if (perspectiveDivide)
	{
		wsPos /= wsPos.w;
	}
	rsmbuffer.Position = wsPos.xyz;

	return rsmbuffer;
}

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
        // TODO: don't hardcore the size
        float2 shadowMapsSize = float2(2048, 2048);

    	float3 shadowPos;
    	float2 shadowUV;
        float2 normalizedUV;

        int cascadeIdx = 3;
        for (int i = 3; i >=0; --i)
        {
    	    float3 pos = mul(float4(gbuffer.Position, 1), dirLight.m_mViewProj[i]).xyz;
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

                normalizedUV = uv;
            }
        }

		for (uint i = 0; i < g_RSMSamplesCount; ++i)
		{
			float2 uvOffset = g_RSMSamplingPattern[i];
			uvOffset *= rsmparams.m_SampleRadius;

			float2 uv = shadowUV + uvOffset;

			RSMBuffer rsmbuffer = RSMBufferDecode(RSMRadiantIntensityTex, RSMNormalTex, shadowMap, samp, uv, normalizedUV, dirLight.m_mInvViewProj[cascadeIdx], false);

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

float3 ShadeDirectionalLightRSM(GBuffer gbuffer, RSMParam rsmparams, DirectionalLight dirLight,
	Texture2D<float4> RSMRadiantIntensityTex, Texture2D<float4> RSMNormalTex, Texture2D<float4> shadowMap, SamplerState samp)
{
	float3 outRadiance = 0.0f;
	if (rsmparams.m_Enabled != 0)
	{
        // TODO: don't hardcore the size
        float2 shadowMapsSize = float2(2048, 2048);

    	float3 shadowPos;
    	float2 shadowUV;
        float2 normalizedUV;

        int cascadeIdx = 3;
        for (int i = 3; i >=0; --i)
        {
    	    float3 pos = mul(float4(gbuffer.Position, 1), dirLight.m_mViewProj[i]).xyz;
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

                normalizedUV = uv;
            }
        }

		for (uint i = 0; i < g_RSMSamplesCount; ++i)
		{
			float2 uvOffset = g_RSMSamplingPattern[i];
			uvOffset *= rsmparams.m_SampleRadius;

			float2 uv = shadowUV + uvOffset;

			RSMBuffer rsmbuffer = RSMBufferDecode(RSMRadiantIntensityTex, RSMNormalTex, shadowMap, samp, uv, normalizedUV, dirLight.m_mInvViewProj[cascadeIdx], false);

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
