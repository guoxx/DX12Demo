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
};

#ifdef __cplusplus
}
#endif

#endif
