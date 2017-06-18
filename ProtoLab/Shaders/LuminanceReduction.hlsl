#include "Inc/Common.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", DescriptorTable(SRV(t0), UAV(u0))" \
RootSigEnd


//=================================================================================================
// Resources
//=================================================================================================
Texture2D<float> ReductionMap : register(t0);
RWTexture2D<float> OutputMap : register(u0);

// -- shared memory
groupshared float LumSamples[LUMINANCE_REDUCTION_NUM_THREADS];


[numthreads(LUMINANCE_REDUCTION_THREAD_GROUP_SIZE, LUMINANCE_REDUCTION_THREAD_GROUP_SIZE, 1)]
RootSigDeclaration
void CSMain(in uint3 GroupID : SV_GroupID,
            in uint3 GroupThreadID : SV_GroupThreadID,
            in uint ThreadIndex : SV_GroupIndex)
{
    uint2 textureSize;
    ReductionMap.GetDimensions(textureSize.x, textureSize.y);

    uint2 samplePos = GroupID.xy * LUMINANCE_REDUCTION_THREAD_GROUP_SIZE + GroupThreadID.xy;
    samplePos = min(samplePos, textureSize - 1);

    float pixelLuminance = ReductionMap[samplePos];

    // Store in shared memory
    LumSamples[ThreadIndex] = pixelLuminance;
    GroupMemoryBarrierWithGroupSync();

    // Sequential Shared Memory proposed by Wolfgang Engel
    // http://diaryofagraphicsprogrammer.blogspot.sg/2014/03/compute-shader-optimizations-for-amd.html
    // Reduce
	[unroll]
	for(uint s = LUMINANCE_REDUCTION_NUM_THREADS / 2; s > 0; s >>= 1)
    {
		if(ThreadIndex < s)
			LumSamples[ThreadIndex] += LumSamples[ThreadIndex + s];

		GroupMemoryBarrierWithGroupSync();
	}

    if(ThreadIndex == 0)
    {
        OutputMap[GroupID.xy] = LumSamples[0] / LUMINANCE_REDUCTION_NUM_THREADS;
    }
}