#include "Inc/Common.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", DescriptorTable(SRV(t0, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)" \
RootSigEnd

HLSLConstantBuffer(SkyConstants, 0, g_Constants);

Texture2D<float4> g_EnvMap: register(t0);

struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 PositionWS: POSITIONWS;
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
    Out.Position = float4(x * 2 - 1, y * 2 - 1, 0.99999, 1);

    float3 ndcPos = float3(Out.Position.xy, 1);
    float4 posWS = mul(float4(ndcPos, 1), g_Constants.m_mInvViewProj);
    Out.PositionWS = posWS.xyz / posWS.w;

    return Out;
}

RootSigDeclaration
float4 PSMain(VSOutput In) : SV_TARGET
{
    float3 dir = normalize(In.PositionWS);
    float theta = acos(dir.y);
    float phi = atan2(dir.z, dir.x);
    if (phi < 0)
        phi += PI*2;

    return g_EnvMap.SampleLevel(g_StaticAnisoWrapSampler, float2((phi / (PI * 2)), theta / PI), 0);
}