#include "Inc/Common.hlsli"
#include "Inc/GBuffer.hlsli"
#include "Inc/Utils.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", SRV(t0, visibility = SHADER_VISIBILITY_VERTEX)" \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", CBV(b1, visibility = SHADER_VISIBILITY_ALL)" \
", DescriptorTable(SRV(t1, numDescriptors=4), visibility=SHADER_VISIBILITY_PIXEL)" \
RootSigEnd


struct VSInput
{
	float3 Position;
	float3 Normal;
	float3 Tangent;
	float3 Bitangent;
	float2 Texcoord;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float3 NormalWS : NORMALWS;
    float3 TangentWS : TANGENTWS;
    float3 BitangentWS : BITANGENTWS;
	float2 Texcoord : TEXCOORD;
	float4 PositionClipSpace : POSITION0;
	float4 PositionClipSpaceLastFrame : POSITION1;
};

struct View
{
	float4x4 mModelViewProj;
	float4x4 mModelViewProjLastFrame;
    float4x4 mModel;
    float4 JitterOffset;
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

HLSLConstantBuffer(View, 0, g_View);
HLSLConstantBuffer(BaseMaterial, 1, g_Material);

StructuredBuffer<VSInput> g_VertexArray : register(t0);
Texture2D<float4> g_AlbedoMap : register(t1);
Texture2D<float4> g_NormalMap : register(t2);
Texture2D<float> g_RoughnessMap : register(t3);
Texture2D<float> g_MetallicMap : register(t4);


RootSigDeclaration
VSOutput VSMain(uint vertid : SV_VertexID)
{
	VSInput In  = g_VertexArray[vertid];

	VSOutput Out;
	Out.Position = mul(float4(In.Position, 1), g_View.mModelViewProj);

    // rotate tangent frame
	Out.NormalWS = mul(float4(In.Normal, 0), g_View.mModel).xyz;
	Out.TangentWS = mul(float4(In.Tangent, 0), g_View.mModel).xyz;
	Out.BitangentWS = mul(float4(In.Bitangent, 0), g_View.mModel).xyz;

	Out.Texcoord = In.Texcoord;

    Out.PositionClipSpace = Out.Position;
    Out.PositionClipSpaceLastFrame = mul(float4(In.Position, 1), g_View.mModelViewProjLastFrame);

	return Out;
}

RootSigDeclaration
GBufferOutput PSMain(VSOutput In)
{
	GBuffer gbuffer;

    float3 albedo = g_AlbedoMap.Sample(g_StaticAnisoWrapSampler, In.Texcoord).xyz;
    float metallic = saturate(g_MetallicMap.Sample(g_StaticAnisoWrapSampler, In.Texcoord));
	gbuffer.Diffuse = lerp(albedo, 0.0f, metallic);
	gbuffer.Specular = lerp(0.03f, albedo, metallic);

	float3 normalTS = float3(0, 0, 1);
    float3 normalWS = normalize(In.NormalWS);
	float3 tangentWS = normalize(In.TangentWS);
	float3 bitangentWS = normalize(In.BitangentWS);
	float3x3 tangentToWorld = float3x3(tangentWS, bitangentWS, normalWS);
    normalTS.xy = g_NormalMap.Sample(g_StaticAnisoWrapSampler, In.Texcoord).xy * 2.0f - 1.0f;
    normalTS.z = sqrt(1.0f - saturate(normalTS.x * normalTS.x + normalTS.y * normalTS.y));
    gbuffer.Normal = normalize(mul(normalTS, tangentToWorld));

    float sqrtRoughness = g_RoughnessMap.Sample(g_StaticAnisoWrapSampler, In.Texcoord);
    gbuffer.Roughness = sqrtRoughness * sqrtRoughness;

    float4 posNdc = In.PositionClipSpace;
    posNdc /= posNdc.w;
    float4 posNdcLastFrame = In.PositionClipSpaceLastFrame;
    posNdcLastFrame /= posNdcLastFrame.w;

    posNdc.xy -= g_View.JitterOffset.xy; 
    posNdcLastFrame.xy -= g_View.JitterOffset.zw;
    float2 velocity = (posNdcLastFrame.xy - posNdc.xy) * float2(0.5, -0.5);
    gbuffer.Velocity = velocity;

	return GBufferEncode(gbuffer);
}