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

#define LUMINANCE_REDUCTION_THREAD_GROUP_SIZE 16
#define LUMINANCE_REDUCTION_NUM_THREADS (LUMINANCE_REDUCTION_THREAD_GROUP_SIZE * LUMINANCE_REDUCTION_THREAD_GROUP_SIZE)

static const int ExposureModes_Manual_SBS = 0;
static const int ExposureModes_Manual_SOS = 1;
static const int ExposureModes_Automatic = 2;

#ifndef __cplusplus
	#if 0
		// TOOD: more elegent solution but not workig on X1, not well supported by FXC
		#define HLSLConstantBuffer(typename_, reg_, var_) \
		ConstantBuffer<typename_> var_ : register(b##reg_);	
	#else
		#define HLSLConstantBuffer(typename_, reg_, var_) \
		cbuffer g_CB##reg_ : register(b##reg_) \
		{ typename_ var_; };
	#endif
#endif

#ifdef __cplusplus
namespace HLSL {
#endif

const static float PI           = 3.141592654f;
const static float DIVPI		= 0.318309886f;

struct RSMParam
{
	int		m_Enabled;
	float	m_SampleRadius;
	float	m_RSMFactor;
	float	m_RadiusEnd;
};

struct EVSMParam
{
	int		m_Enabled;
	float	m_PositiveExponent;
	float	m_NegativeExponent;
	float	m_LightBleedingReduction;
	float	m_VSMBias;
	float	m_Padding0;
	float	m_Padding1;
	float	m_Padding2;
};

struct PointLight
{
	float4		m_PositionAndRadius;
	float4		m_RadiantPower;
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
	float		m_Padding0;
	float3		m_Irradiance;
	float		m_Padding1;
	float4x4	m_mViewProj[4];
	float4x4	m_mInvViewProj[4];
	// -1 means invalid
	int			m_ShadowMapTexId;
	int			m_RSMIntensityTexId;
	int			m_RSMNormalTexId;
	int			m_EVSMTexId;
};

struct Camera
{
	float3		m_Position;
	float		m_Padding0;
	float4x4	m_mViewProj;
	float4x4	m_mInvProj;
	float4x4	m_mInvViewProj;
};

struct LightNode
{
	int			m_LightIndex;
};

struct LightCullingConstants
{
	uint m_NumPointLights;
	uint m_NumTileX;
	uint m_NumTileY;
	uint m_Padding0;
	float4x4 m_mView;
	float4x4 m_mInvProj;
	float4 m_InvScreenSize;
};

struct TiledShadingConstants
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

	RSMParam	m_RSM;
	EVSMParam	m_EVSM;
};

struct BaseMaterialRSMConstants
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
};

struct DirectionalLightConstants
{
	float4x4 mInvView;
	float4x4 mInvProj;
    float4x4 mInvViewProj;
	float4 CameraPosition;

	DirectionalLight	m_DirLight;
	RSMParam			m_RSM;
	EVSMParam			m_EVSM;
};

struct EVSMConstants
{
	EVSMParam			m_EVSM;
};

struct CameraSettings
{
    int m_ToneMapEnabled;
    int m_ExposureMode;
    float m_Aperture;
    float m_ShutterSpeed;
    float m_ISO;

    float m_Dummy0;
    float m_Dummy1;
    float m_Dummy2;
};

#ifdef __cplusplus
}
#endif

#endif
