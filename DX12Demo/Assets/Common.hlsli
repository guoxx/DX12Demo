struct View
{
	float4x4 mModelViewProj;
};

ConstantBuffer<BaseMaterial> g_View : register(b0)

#define RootSigElemViewCBV \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)"


#define RootSigBegin \
[RootSignature("RootFlags(0)" \
RootSigElemViewCBV

#define RootSigEnd \
)]


