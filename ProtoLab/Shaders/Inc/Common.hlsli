#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

#include "HLSLShared.h"

#define STATIC_SAMPLERS_SIGNATURE \
", StaticSampler(s0, filter=FILTER_MIN_MAG_MIP_POINT,  addressU=TEXTURE_ADDRESS_CLAMP, addressV=TEXTURE_ADDRESS_CLAMP, addressW=TEXTURE_ADDRESS_CLAMP, visibility=SHADER_VISIBILITY_ALL)" \
", StaticSampler(s1, filter=FILTER_MIN_MAG_MIP_LINEAR, addressU=TEXTURE_ADDRESS_CLAMP, addressV=TEXTURE_ADDRESS_CLAMP, addressW=TEXTURE_ADDRESS_CLAMP, visibility=SHADER_VISIBILITY_ALL)" \
", StaticSampler(s2, filter=FILTER_ANISOTROPIC,        addressU=TEXTURE_ADDRESS_CLAMP, addressV=TEXTURE_ADDRESS_CLAMP, addressW=TEXTURE_ADDRESS_CLAMP, visibility=SHADER_VISIBILITY_ALL)" \
", StaticSampler(s3, filter=FILTER_MIN_MAG_MIP_POINT,  addressU=TEXTURE_ADDRESS_WRAP,  addressV=TEXTURE_ADDRESS_WRAP,  addressW=TEXTURE_ADDRESS_WRAP,  visibility=SHADER_VISIBILITY_ALL)" \
", StaticSampler(s4, filter=FILTER_MIN_MAG_MIP_LINEAR, addressU=TEXTURE_ADDRESS_WRAP,  addressV=TEXTURE_ADDRESS_WRAP,  addressW=TEXTURE_ADDRESS_WRAP,  visibility=SHADER_VISIBILITY_ALL)" \
", StaticSampler(s5, filter=FILTER_ANISOTROPIC,        addressU=TEXTURE_ADDRESS_WRAP,  addressV=TEXTURE_ADDRESS_WRAP,  addressW=TEXTURE_ADDRESS_WRAP,  visibility=SHADER_VISIBILITY_ALL)" 

SamplerState g_StaticPointClampSampler : register(s0);
SamplerState g_StaticLinearClampSampler : register(s1);
SamplerState g_StaticAnisoClampSampler : register(s2);

SamplerState g_StaticPointWrapSampler : register(s3);
SamplerState g_StaticLinearWrapSampler : register(s4);
SamplerState g_StaticAnisoWrapSampler : register(s5);


#define RootSigBegin \
[RootSignature("RootFlags(0)"

#define RootSigEnd \
STATIC_SAMPLERS_SIGNATURE \
)]

#endif
