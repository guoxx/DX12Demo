
#define RootSigStart \
[RootSignature("RootFlags(0)"

#define RootSigEnd \
)]

#define RootSigElemVertexArray \
", SRV(t0, visibility = SHADER_VISIBILITY_VERTEX)"


StructuredBuffer<VSInput> g_VertexArray : register(t0);
