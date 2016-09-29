
#define RootSig \
"RootFlags(0)" \
", RootConstants(b0, num32BitConstants=1, visibility=SHADER_VISIBILITY_VERTEX)" \
", SRV(t0, visibility = SHADER_VISIBILITY_VERTEX)"

cbuffer View : register(b0)
{
	float g_Offset;
};

struct VSInput
{
	float3 Position : POSITION;
};

StructuredBuffer<VSInput> g_VertexArray : register(t0);

struct VSOutput
{
	float4 Position : SV_POSITION;
};

[RootSignature(RootSig)]
VSOutput VSMain(uint vertid : SV_VertexID)
{
	VSInput In  = g_VertexArray[vertid];

	VSOutput Out;
	Out.Position = float4(In.Position, 1);
	Out.Position.x += g_Offset;
	return Out;
}

[RootSignature(RootSig)]
float4 PSMain(VSOutput In) : SV_TARGET
{
	return float4(0.5f, 0.0f, 0.0f, 1.0f);
}