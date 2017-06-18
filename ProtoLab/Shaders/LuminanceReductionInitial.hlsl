#include "Inc/Common.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", DescriptorTable(SRV(t0), UAV(u0))" \
RootSigEnd


//=================================================================================================
// Resources
//=================================================================================================
Texture2D<float4> InputMap : register(t0);
RWTexture2D<float> OutputMap : register(u0);

// -- shared memory
groupshared float LumSamples[LUMINANCE_REDUCTION_NUM_THREADS];

// Calculates luminance from an RGB value
float CalcLuminance(float3 color)
{
    return dot(color, float3(0.2126f, 0.7152f, 0.0722f));
}

//=================================================================================================
// Luminance reduction, intial pass
//=================================================================================================
[numthreads(LUMINANCE_REDUCTION_THREAD_GROUP_SIZE, LUMINANCE_REDUCTION_THREAD_GROUP_SIZE, 1)]
RootSigDeclaration
void CSMain(in uint3 GroupID : SV_GroupID,
            in uint3 GroupThreadID : SV_GroupThreadID,
            uint ThreadIndex : SV_GroupIndex)
{
    uint2 textureSize;
    InputMap.GetDimensions(textureSize.x, textureSize.y);

    uint2 samplePos = GroupID.xy * LUMINANCE_REDUCTION_THREAD_GROUP_SIZE + GroupThreadID.xy;
    samplePos = min(samplePos, textureSize - 1);

    float3 colorSample = InputMap[samplePos].xyz;

    float lumSample = max(CalcLuminance(colorSample), 0.00001f);
    float pixelLuminance = log(lumSample);

    // -- store in shared memory
    LumSamples[ThreadIndex] = pixelLuminance;
    GroupMemoryBarrierWithGroupSync();

    // -- reduce
	[unroll]
	for(uint s = LUMINANCE_REDUCTION_NUM_THREADS / 2; s > 0; s >>= 1)
    {
		if(ThreadIndex < s)
			LumSamples[ThreadIndex] += LumSamples[ThreadIndex + s];

		GroupMemoryBarrierWithGroupSync();
	}

    if(ThreadIndex == 0)
        OutputMap[GroupID.xy] = LumSamples[0] / LUMINANCE_REDUCTION_NUM_THREADS;
}
