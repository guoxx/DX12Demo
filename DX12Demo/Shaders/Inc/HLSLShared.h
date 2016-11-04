#ifndef __HLSL_SHARED_H__
#define __HLSL_SHARED_H__

#ifdef __cplusplus
	using float4x4 = DirectX::XMFLOAT4X4;
	using float4 = DirectX::XMFLOAT4;
	using float3 = DirectX::XMFLOAT3;
	using float2 = DirectX::XMFLOAT2;

	using int4 = DirectX::XMINT4;
	using int3 = DirectX::XMINT3;
	using int2 = DirectX::XMINT2;

	using uint4 = DirectX::XMUINT4;
	using uint3 = DirectX::XMUINT3;
	using uint2 = DirectX::XMUINT2;
	using uint = uint32_t;
#endif

#define MAX_DIRECTIONAL_LIGHTS_PER_FRAME 4
#define MAX_POINT_LIGHTS_PER_FRAME 1024
#define MAX_LIGHT_NODES_PER_TILE 128

#define LIGHT_CULLING_NUM_THREADS_XY 8
#define NUM_THREADS_PER_LIGHT_CULLING_TILE (LIGHT_CULLING_NUM_THREADS_XY * LIGHT_CULLING_NUM_THREADS_XY)
#define LIGHT_NODE_INVALID 0x7FFFFFFF

// internal use only
#define __HLSL_CB_TYPE_NAME__(namespace_, typename_) namespace_##typename_
#define __HLSL_CB_VAR_NAME__(reg_) g_CB##reg_
#define __HLSL_CB_REG_NAME__(reg_) register(b##reg_)

#ifdef __cplusplus
	#define HLSL_CB_DECL(namespace_, typename_, reg_, decl_) \
	struct __declspec(align(16)) __HLSL_CB_TYPE_NAME__(namespace_, typename_) \
	decl_;
#else
	#if 0
		// TOOD: more elegent solution but not workig on X1, not well supported by FXC
		#define HLSL_CB_DECL(namespace_, typename_, reg_, decl_) \
		struct __HLSL_CB_TYPE_NAME__(namespace_, typename_) \
		decl_; \
		ConstantBuffer<__HLSL_CB_TYPE_NAME__(namespace_, typename_)> __HLSL_CB_VAR_NAME__(reg_) : __HLSL_CB_REG_NAME__(reg_);	

		#define HLSL_CB_GET(reg_, member_) \
		__HLSL_CB_VAR_NAME__(reg_).member_
	#else
		#define HLSL_CB_DECL(namespace_, typename_, reg_, decl_) \
		cbuffer __HLSL_CB_VAR_NAME__(reg_) : __HLSL_CB_REG_NAME__(reg_) \
		decl_;

		#define HLSL_CB_GET(reg_, member_) \
		member_
	#endif
#endif

#ifdef __cplusplus
namespace HLSL {
#endif

const static float PI           = 3.141592654f;
const static float DIVPI		= 0.318309886f;

struct PointLight
{
	float4		m_Position;
	 // x: radius start, y : radius end
	float4		m_RadiusParam;
	float4		m_Intensity;
	float4x4	m_mViewProj[6];
	//  x: first texture index of 6 shadow maps per point light, -1 means invalid
	int			m_FirstShadowMapTexId;
	int			m_Padding0;
	int			m_Padding1;
	int			m_Padding2;
};

struct DirectionalLight
{
	float3		m_Direction;
	// -1 means invalid
	int			m_ShadowMapTexId;
	float4		m_Irradiance;
	float4x4	m_mViewProj;
};

struct LightNode
{
	int			m_LightIndex;
};

#ifdef __cplusplus
HLSL_CB_DECL(TiledShading, Constants, 0,
{
	uint m_NumTileX;
	uint m_NumTileY;
	uint m_ScreenWidth;
	uint m_ScreenHeight;
	uint m_NumDirectionalLights;
	uint m_Padding0;
	uint m_Padding1;
	uint m_Padding2;
	float4 m_CameraPosition;
	float4x4 m_mInvView;
	float4x4 m_mInvProj;
});

HLSL_CB_DECL(BaseMaterial_RSM, Constants, 0,
{
	float4x4 mModelViewProj;
	float4x4 mInverseTransposeModel;

	int LightType;
	int Padding0;
	int Padding1;
	int Padding2;

	float4 DirectionalLightIrradiance;
	float4 DirectionalLightDirection;
	float4 PointLightIntensity;
	float4 PointLightPosition;
});

HLSL_CB_DECL(DirectionalLight, Constants, 0,
{
	float4x4 mInvView;
	float4x4 mInvProj;
	float4 LightDirection;
	float4 LightIrradiance;
	float4 CameraPosition;
	float4x4 mLightViewProj;
	float4x4 mLightInvViewProj;

	int RSMEnabled;
	float RSMSampleRadius;
	float RSMSampleWeight;
	float RSMRadiusThreshold;
});
#endif

#ifdef __cplusplus
}
#endif

#endif
