#ifndef __UTILS_HLSLI__
#define __UTILS_HLSLI__

// index of refraction to F0
float IorToF0_Dielectric(float ior)
{
	return pow(ior - 1, 2) / pow(ior + 1, 2);
}

float ShininessToRoughness(float r)
{
    // equation proposed from http://graphicrants.blogspot.sg/2013/08/specular-brdf-reference.html
    return sqrt(2.0 / (r + 2.0));
}

const static uint g_RSMSamplesCount = 64;
const static float2 g_RSMSamplingPattern[64] = {
float2(-0.3579344f, -0.3517149f),
float2(-0.3890468f, -0.7202561f),
float2(-0.2567423f, -0.0009548062f),
float2(-0.2508792f, -0.5212535f),
float2(-0.4917447f, -0.0206038f),
float2(-0.5943144f, -0.310339f),
float2(-0.08068617f, -0.2073122f),
float2(-0.5964261f, -0.6146008f),
float2(-0.06257999f, -0.4119583f),
float2(-0.003197781f, -0.6396211f),
float2(-0.1764597f, -0.8859963f),
float2(0.2531657f, -0.4200461f),
float2(0.05629459f, -0.9949071f),
float2(0.166944f, -0.7750345f),
float2(0.3332334f, -0.6005474f),
float2(0.2902015f, -0.9262927f),
float2(0.472591f, -0.8016518f),
float2(-0.7654607f, -0.4435649f),
float2(-0.5578633f, -0.8274426f),
float2(-0.5786387f, 0.1987577f),
float2(0.07728124f, 0.04528601f),
float2(-0.2269627f, 0.2174418f),
float2(-0.8871316f, -0.1302793f),
float2(-0.7454758f, -0.0006463315f),
float2(-0.9457548f, -0.3118609f),
float2(-0.001683236f, 0.2770302f),
float2(0.2118009f, -0.1378248f),
float2(0.509697f, -0.02626915f),
float2(0.3068107f, 0.1078179f),
float2(0.4372152f, -0.2751352f),
float2(0.2258023f, 0.3498218f),
float2(0.7067862f, -0.3505104f),
float2(0.4158016f, 0.2694948f),
float2(0.6265055f, 0.1402356f),
float2(0.8327943f, 0.02783612f),
float2(0.6864228f, -0.1073536f),
float2(0.6737096f, -0.6840767f),
float2(0.490763f, -0.4835717f),
float2(-0.08402723f, 0.4619588f),
float2(0.3806773f, 0.6527942f),
float2(0.1396271f, 0.5422848f),
float2(0.5649002f, 0.4364094f),
float2(0.8278279f, -0.5233458f),
float2(0.9161091f, -0.3291859f),
float2(0.7785613f, 0.3681582f),
float2(0.1783977f, 0.8567262f),
float2(0.5152924f, 0.8373909f),
float2(0.7113628f, 0.6190548f),
float2(0.01140799f, 0.7131786f),
float2(-0.9841616f, 0.1477079f),
float2(-0.7976238f, 0.2312148f),
float2(-0.693472f, 0.406074f),
float2(-0.3649378f, 0.4050539f),
float2(-0.5691569f, 0.5673698f),
float2(-0.5047651f, 0.7472093f),
float2(-0.2069313f, 0.7437413f),
float2(0.9562784f, 0.2554863f),
float2(0.9646869f, -0.1418799f),
float2(-0.4015996f, 0.9155728f),
float2(-0.8067811f, 0.5761073f),
float2(0.0941247f, -0.290804f),
float2(-0.05625709f, 0.9482339f),
float2(-0.3657628f, 0.6047338f),
float2(-0.3679701f, -0.9156333f),
};

#endif
