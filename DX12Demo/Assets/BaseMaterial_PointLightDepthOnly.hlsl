#include "Common.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", SRV(t0, visibility = SHADER_VISIBILITY_VERTEX)" \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
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
};

struct GSOutput
{
	float4 Position : SV_POSITION;
	uint RTIndex : SV_RENDERTARGETARRAYINDEX;
};

struct View
{
	float4x4 mModelViewProj[6];
};

ConstantBuffer<View> g_View : register(b0);
StructuredBuffer<VSInput> g_VertexArray : register(t0);


RootSigDeclaration
VSOutput VSMain(uint vertid : SV_VertexID)
{
	VSInput In  = g_VertexArray[vertid];

	VSOutput Out;
	Out.Position = float4(In.Position, 1);

	return Out;
}

[maxvertexcount(18)]
RootSigDeclaration
void GSMain(triangle VSOutput In[3], inout TriangleStream<GSOutput> OutStream)
{
	for (uint f = 0; f < 6; ++f)
	{
		for (uint i = 0; i < 3; ++i)
		{
			GSOutput result;
			result.RTIndex = f;
			result.Position = mul(In[i].Position, g_View.mModelViewProj[f]);
			OutStream.Append(result);
		}
		OutStream.RestartStrip();
	}
}