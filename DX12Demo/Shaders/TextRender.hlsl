#define Text_RootSig \
	"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
	"CBV(b0, visibility = SHADER_VISIBILITY_VERTEX)," \
	"CBV(b0, visibility = SHADER_VISIBILITY_PIXEL)," \
	"DescriptorTable(SRV(t0, numDescriptors = 1), visibility = SHADER_VISIBILITY_PIXEL)," \
	"StaticSampler(s0, visibility = SHADER_VISIBILITY_PIXEL," \
		"addressU = TEXTURE_ADDRESS_CLAMP," \
		"addressV = TEXTURE_ADDRESS_CLAMP," \
		"addressW = TEXTURE_ADDRESS_CLAMP," \
		"filter = FILTER_MIN_MAG_MIP_LINEAR)"


cbuffer cbFontParams : register( b0 )
{
	float2 Scale;			// Scale and offset for transforming coordinates
	float2 Offset;
	float2 InvTexDim;		// Normalizes texture coordinates
	float TextSize;			// Height of text in destination pixels
	float TextScale;		// TextSize / FontHeight
	float DstBorder;		// Extra space around a glyph measured in screen space coordinates
	uint SrcBorder;			// Extra spacing around glyphs to avoid sampling neighboring glyphs
}

struct VS_INPUT
{
	float2 ScreenPos : POSITION;	// Upper-left position in screen pixel coordinates
	uint4  Glyph : TEXCOORD;		// X, Y, Width, Height in texel space
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;	// Upper-left and lower-right coordinates in clip space
	float2 Tex : TEXCOORD0;		// Upper-left and lower-right normalized UVs
};

[RootSignature(Text_RootSig)]
VS_OUTPUT VSMain( VS_INPUT input, uint VertID : SV_VertexID )
{
	const float2 xy0 = input.ScreenPos - DstBorder;
	const float2 xy1 = input.ScreenPos + DstBorder + float2(TextScale * input.Glyph.z, TextSize);
	const uint2 uv0 = input.Glyph.xy - SrcBorder;
	const uint2 uv1 = input.Glyph.xy + SrcBorder + input.Glyph.zw;

	float2 uv = float2( VertID & 1, (VertID >> 1) & 1 );

	VS_OUTPUT output;
	output.Pos = float4( lerp(xy0, xy1, uv) * Scale + Offset, 0, 1 );
	output.Tex = lerp(uv0, uv1, uv) * InvTexDim;
	return output;
}

cbuffer cbFontParams : register( b0 )
{
	float4 Color;
	float2 ShadowOffset;
	float ShadowHardness;
	float ShadowOpacity;
	float HeightRange;	// The range of the signed distance field.
}

Texture2D<float> SignedDistanceFieldTex : register( t0 );
SamplerState LinearSampler : register( s0 );

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float GetAlpha( float2 uv )
{
	return saturate(SignedDistanceFieldTex.Sample(LinearSampler, uv) * HeightRange + 0.5);
}

[RootSignature(Text_RootSig)]
float4 PSMain( PS_INPUT Input ) : SV_Target
{
	return float4(Color.rgb, 1) * GetAlpha(Input.uv) * Color.a;
}