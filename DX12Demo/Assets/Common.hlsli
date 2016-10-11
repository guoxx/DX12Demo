
#define RootSigBegin \
[RootSignature("RootFlags(0)"

#define RootSigEnd \
)]


// G-Bufer Layout
/*

+----------------+----------------+----------------+----------------+
| diffuse                                          +                +
+----------------+----------------+----------------+----------------+
| specular                                         +                +
+----------------+----------------+----------------+----------------+
| normal                                           + roughness      +
+----------------+----------------+----------------+----------------+

*/

struct GBufferOutput
{
	float4 Diffuse          : SV_TARGET0;
	float4 Specular         : SV_TARGET1;
	float4 Normal_Roughness : SV_TARGET2;
};

// index of refraction to F0
float IorToF0_Dielectric(float ior)
{
	return pow(ior - 1, 2) / pow(ior + 1, 2);
}
