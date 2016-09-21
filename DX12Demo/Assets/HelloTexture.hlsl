
Texture2D<float4> g_TextureArray[] : register(t0);
SamplerState g_Sampler : register(s0);

struct VSInput
{
	float3 Position : POSITION;
	float2 UV0 : TEXCOORD0;
	int TexID : TEXID;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 UV0 : TEXCOORD0;
	nointerpolation int TexID : TEXID;
};

VSOutput VSMain(VSInput In)
{
	VSOutput Out;
	Out.Position = float4(In.Position, 1);
	Out.UV0 = In.UV0;
	Out.TexID = In.TexID;
	return Out;
}

float4 PSMain(VSOutput In) : SV_TARGET
{
	float4 c = g_TextureArray[In.TexID].Sample(g_Sampler, In.UV0);
	return float4(c.rgb, 1.0f);
}