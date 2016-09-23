
#define RootSig \
"RootFlags(0)" \
", DescriptorTable(SRV(t0, numDescriptors=2), visibility=SHADER_VISIBILITY_PIXEL)" \
", DescriptorTable(Sampler(s0, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)"

Texture2D<float4> g_TextureArray[2] : register(t0);
SamplerState g_Sampler : register(s0);

struct VSInput
{
	float3 Position : POSITION;
	float2 UV0 : TEXCOORD0;
	uint TexID : TEXID;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 UV0 : TEXCOORD0;
	nointerpolation uint TexID : TEXID;
};

[RootSignature(RootSig)]
VSOutput VSMain(VSInput In)
{
	VSOutput Out;
	Out.Position = float4(In.Position, 1);
	Out.UV0 = In.UV0;
	Out.TexID = In.TexID;
	return Out;
}

[RootSignature(RootSig)]
float4 PSMain(VSOutput In) : SV_TARGET
{
	float4 c;
	uint texID = In.TexID % 2;
	if (texID == 0)
	{
		c = g_TextureArray[0].Sample(g_Sampler, In.UV0);
	}
	else
	{
		c = g_TextureArray[1].Sample(g_Sampler, In.UV0);
	}
	return float4(c.rgb, 1.0f);
}