#include "Inc/Common.hlsli"
#include "Inc/Exposure.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", DescriptorTable(SRV(t0, numDescriptors=2), visibility=SHADER_VISIBILITY_PIXEL)" \
RootSigEnd

Texture2D<float4> g_Texture : register(t0);
Texture2D<float> g_AvgLuminanceTex : register(t1);

HLSLConstantBuffer(CameraSettings, 0, g_CameraSettings);

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

float GetAvgLuminance(Texture2D<float> lumTex)
{
    return lumTex.Load(uint3(0, 0, 0)).x;
}

float3 ACESFilm( float3 x )
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}

float3 ToneMapFilmicALU(float3 color)
{
    color = max(0, color - 0.004f);
    color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f)+ 0.06f);

    // result has 1/2.2 baked in
    return pow(color, 2.2f);
}

RootSigDeclaration
float4 PSMain(VSOutput In) : SV_TARGET
{
	float4 hdrColor = g_Texture.Sample(g_StaticPointClampSampler, In.Texcoord);
    float avgLuminance = GetAvgLuminance(g_AvgLuminanceTex);
    float exposure;
    float3 exposuredColor = CalcExposedColor(g_CameraSettings, hdrColor.xyz, avgLuminance, 0, exposure);
    float3 sdrColor = ToneMapFilmicALU(exposuredColor);

	return float4(sdrColor, saturate(hdrColor.a));
}