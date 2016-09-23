
#define RootSig \
"RootFlags(0)" \
", RootConstants(num32BitConstants=1, b0, visibility=SHADER_VISIBILITY_VERTEX)"

cbuffer View : register(b0)
{
	float g_Offset;
};

struct VSInput
{
	float3 Position : POSITION;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
};

[RootSignature(RootSig)]
VSOutput VSMain(VSInput In)
{
	VSOutput Out;
	Out.Position = float4(In.Position, 1);
	Out.Position.x += g_Offset;
	return Out;
}

[RootSignature(RootSig)]
float4 PSMain(VSOutput In) : SV_TARGET
{
	return float4(pow(0.5f, 2.2f), 0.0f, 0.0f, 1.0f);
}