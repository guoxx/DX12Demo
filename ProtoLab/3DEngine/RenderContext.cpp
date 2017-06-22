#include "pch.h"
#include "RenderContext.h"


RenderContext::RenderContext()
{
	m_ShadingCfg = ShadingConfiguration_GBuffer;
}

RenderContext::~RenderContext()
{
}

RenderableSurfaceHandle<DX12ColorSurface> RenderContext::AcquireRSMRadiantIntensitySurfaceForDirectionalLight(const DirectionalLight * pDirLight)
{
	RenderableSurfaceHandle<DX12ColorSurface> handle;

	auto result = m_RSMRadiantIntensitySurfaceForDirLights.find(pDirLight);
	if (result != m_RSMRadiantIntensitySurfaceForDirLights.end())
	{
		handle = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_R8G8B8A8_UNORM, DX12DirectionalLightShadowMapSize, DX12DirectionalLightShadowMapSize };
		handle = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc);
		m_RSMRadiantIntensitySurfaceForDirLights.insert(std::make_pair(pDirLight, handle));
	}

	return handle;
}

RenderableSurfaceHandle<DX12ColorSurface> RenderContext::AcquireRSMNormalSurfaceForDirectionalLight(const DirectionalLight * pDirLight)
{
	RenderableSurfaceHandle<DX12ColorSurface> handle;

	auto result = m_RSMNormalSurfaceForDirLights.find(pDirLight);
	if (result != m_RSMNormalSurfaceForDirLights.end())
	{
		handle = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_R8G8B8A8_UNORM, DX12DirectionalLightShadowMapSize, DX12DirectionalLightShadowMapSize };
		handle = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc);
		m_RSMNormalSurfaceForDirLights.insert(std::make_pair(pDirLight, handle));
	}

	return handle;
}

RenderableSurfaceHandle<DX12ColorSurface> RenderContext::AcquireEVSMSurfaceForDirectionalLight(const DirectionalLight * pDirLight)
{
	RenderableSurfaceHandle<DX12ColorSurface> handle;

	auto result = m_EVSMSurfaceForDirLights.find(pDirLight);
	if (result != m_EVSMSurfaceForDirLights.end())
	{
		handle = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_R32G32B32A32_FLOAT, DX12DirectionalLightShadowMapSize, DX12DirectionalLightShadowMapSize };
		handle = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc);
		m_EVSMSurfaceForDirLights.insert(std::make_pair(pDirLight, handle));
	}

	return handle;
}

RenderableSurfaceHandle<DX12DepthSurface>  RenderContext::AcquireDepthSurfaceForDirectionalLight(const DirectionalLight* pDirLight)
{
	RenderableSurfaceHandle<DX12DepthSurface> handle;

	auto result = m_ShadowMapForDirLights.find(pDirLight);
	if (result != m_ShadowMapForDirLights.end())
	{
		handle = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_D32_FLOAT, DX12DirectionalLightShadowMapSize, DX12DirectionalLightShadowMapSize };
		handle = RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(desc);
		m_ShadowMapForDirLights.insert(std::make_pair(pDirLight, handle));
	}

	return handle;
}

std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6> RenderContext::AcquireEVSMSurfaceForPointLight(const PointLight * pPointLight)
{
	std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6> handles;

	auto result = m_EVSMSurfaceForPointLights.find(pPointLight);
	if (result != m_EVSMSurfaceForPointLights.end())
	{
		handles = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_R32G32B32A32_FLOAT, DX12PointLightShadowMapSize, DX12PointLightShadowMapSize };
		handles = { 
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
		};
		m_EVSMSurfaceForPointLights.insert(std::make_pair(pPointLight, handles));
	}

	return handles;
}

std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6> RenderContext::AcquireRSMRadiantIntensitySurfaceForPointLight(const PointLight * pPointLight)
{
	std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6> handles;

	auto result = m_RSMRadiantIntensitySurfaceForPointLights.find(pPointLight);
	if (result != m_RSMRadiantIntensitySurfaceForPointLights.end())
	{
		handles = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_R8G8B8A8_UNORM, DX12PointLightShadowMapSize, DX12PointLightShadowMapSize };
		handles = { 
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
		};
		m_RSMRadiantIntensitySurfaceForPointLights.insert(std::make_pair(pPointLight, handles));
	}

    return handles;
}

std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6> RenderContext::AcquireRSMNormalSurfaceForPointLight(const PointLight * pPointLight)
{
	std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6> handles;

	auto result = m_RSMNormalSurfaceForPointLights.find(pPointLight);
	if (result != m_RSMNormalSurfaceForPointLights.end())
	{
		handles = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_R8G8B8A8_UNORM, DX12PointLightShadowMapSize, DX12PointLightShadowMapSize };
		handles = { 
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireColorSurface(desc),
		};
		m_RSMNormalSurfaceForPointLights.insert(std::make_pair(pPointLight, handles));
	}

    return handles;
}

std::array<RenderableSurfaceHandle<DX12DepthSurface>, 6> RenderContext::AcquireDepthSurfaceForPointLight(const PointLight* pPointLight)
{
	std::array<RenderableSurfaceHandle<DX12DepthSurface>, 6> handles;

	auto result = m_ShadowMapForPointLights.find(pPointLight);
	if (result != m_ShadowMapForPointLights.end())
	{
		handles = result->second;
	}
	else
	{
		RenderableSurfaceDesc desc{ GFX_FORMAT_D32_FLOAT, DX12PointLightShadowMapSize, DX12PointLightShadowMapSize };
		handles = { 
			RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(desc),
			RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(desc),
		};
		m_ShadowMapForPointLights.insert(std::make_pair(pPointLight, handles));
	}

    return handles;
}

void RenderContext::ReleaseDepthSurfacesForAllLights()
{
	// TODO
}
