#include "Inc/Common.hlsli"
#include "Inc/VSM.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", DescriptorTable(SRV(t0, numDescriptors=3), visibility=SHADER_VISIBILITY_PIXEL)" \
RootSigEnd

HLSLConstantBuffer(DirectionalLightConstants, 0, g_Constants);

Texture2D<float> g_DepthTexture : register(t0);
Texture2D<float> g_ShadowMap : register(t1);
Texture2D<float4> g_EVSMTexture : register(t2);

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

float ShadowMask(float3 positionWS)
{
    // TODO: don't hardcore the size
    float2 shadowMapsSize = float2(2048, 2048);

	float3 shadowPos[4];
    int cascadeIdx = 3;

    for (int i = 3; i >=0; --i)
    {
	    float3 pos = mul(float4(positionWS, 1), g_Constants.m_DirLight.m_mViewProj[i]).xyz;
        shadowPos[i] = pos;

        float border = 4;
        if (all(abs(pos.xy) < ((shadowMapsSize / 4 - border) / (shadowMapsSize / 4))))
        {
            cascadeIdx = i;
        }
    }

	// trick come from https://forum.beyond3d.com/threads/variance-shadow-maps-demo-d3d10.37062/#post-980601
	const int SplitPowLookup[8] = {0, 1, 1, 2, 2, 2, 2, 3};
    
	// Ensure that every fragment in the quad choses the same split so that
	// derivatives will be meaningful for proper texture filtering and LOD selection.
	int SplitPow = 1 << cascadeIdx;
	int SplitX = abs(ddx(SplitPow));
	int SplitY = abs(ddy(SplitPow));
	int SplitXY = abs(ddx(SplitY));
	int SplitMax = max(SplitXY, max(SplitX, SplitY));
    if (SplitMax > 0)
    {
        cascadeIdx = SplitPowLookup[SplitMax-1];
    }

	float2 shadowUV;
    float2 uv = float2((shadowPos[cascadeIdx].x + 1.0f) * 0.5f, (-shadowPos[cascadeIdx].y + 1.0f) * 0.5f);
    int tx = cascadeIdx & 0x01;
    int ty = (cascadeIdx & 0x02) > 1;
    shadowUV.x = tx * 0.5 + uv.x * 0.5;
    shadowUV.y = ty * 0.5 + uv.y * 0.5;

	float shadowMask = 1.0f;
	if (g_Constants.m_EVSM.m_Enabled)
	{
		float2 exponents = GetEVSMExponents(g_Constants.m_EVSM.m_PositiveExponent, g_Constants.m_EVSM.m_NegativeExponent, SMFormat32Bit);
		float2 warpedDepth = WarpDepth(shadowPos[cascadeIdx].z, exponents);

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
		shadowMask = occluderDepth < shadowPos[cascadeIdx].z ? 0.0f : 1.0f;
	}
	return shadowMask;
}

RootSigDeclaration
float4 PSMain(VSOutput In) : SV_TARGET
{
    float depth = g_DepthTexture.SampleLevel(g_StaticPointClampSampler, In.Texcoord, 0);

	float3 ndcPos = float3(In.Texcoord * (2, -2) + (-1, 1), depth);
	float4 positionWS = mul(float4(ndcPos, 1), g_Constants.mInvViewProj);
	positionWS /= positionWS.w;

    return ShadowMask(positionWS);
}