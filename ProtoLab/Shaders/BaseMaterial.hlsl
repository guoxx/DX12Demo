#include "Inc/Common.hlsli"
#include "Inc/GBuffer.hlsli"
#include "Inc/Utils.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", SRV(t0, visibility = SHADER_VISIBILITY_VERTEX)" \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", CBV(b1, visibility = SHADER_VISIBILITY_ALL)" \
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
	float4 PositionClipSpace : POSITION0;
	float4 PositionClipSpaceLastFrame : POSITION1;
};

struct View
{
	float4x4 mModelViewProj;
	float4x4 mModelViewProjLastFrame;
	float4x4 mInverseTransposeModel;
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
Texture2D<float4> g_DiffuseTexture : register(t1);


RootSigDeclaration
VSOutput VSMain(uint vertid : SV_VertexID)
{
	VSInput In  = g_VertexArray[vertid];

	VSOutput Out;
	Out.Position = mul(float4(In.Position, 1), g_View.mModelViewProj);
	Out.Normal = mul(float4(In.Normal, 0), g_View.mInverseTransposeModel).xyz;
	Out.Texcoord = In.Texcoord;

    Out.PositionClipSpace = Out.Position;
    Out.PositionClipSpaceLastFrame = mul(float4(In.Position, 1), g_View.mModelViewProjLastFrame);

	return Out;
}

RootSigDeclaration
GBufferOutput PSMain(VSOutput In)
{
	GBuffer gbuffer;

	gbuffer.Diffuse = g_DiffuseTexture.Sample(g_StaticAnisoWrapSampler, In.Texcoord).xyz;
	gbuffer.Specular = IorToF0_Dielectric(g_Material.Ior.x).xxx;
	gbuffer.Normal = In.Normal;
	gbuffer.Roughness = saturate((100.0f - g_Material.Shininess.x) / 100.0f);

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