
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

VSOutput VSMain(VSInput In)
{
	VSOutput Out;
	Out.Position = float4(In.Position, 1);
	Out.Position.x += g_Offset;
	return Out;
}

float4 PSMain(VSOutput In) : SV_TARGET
{
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}