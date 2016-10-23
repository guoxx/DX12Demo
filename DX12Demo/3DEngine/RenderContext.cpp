#include "pch.h"
#include "RenderContext.h"


RenderContext::RenderContext()
{
	m_ShadingCfg = ShadingConfiguration_GBuffer;
}

RenderContext::~RenderContext()
{
}

DX12DepthSurface*  RenderContext::AcquireDepthSurfaceForDirectionalLight(DirectionalLight* pDirLight)
{
	RenderableSurfaceHandle handle;

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

	return RenderableSurfaceManager::GetInstance()->GetDepthSurface(handle);
}

std::array<DX12DepthSurface*, 6> RenderContext::AcquireDepthSurfaceForPointLight(PointLight* pPointLight)
{
	std::array<RenderableSurfaceHandle, 6> handles;

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

	return std::array<DX12DepthSurface*, 6>{
			RenderableSurfaceManager::GetInstance()->GetDepthSurface(handles[0]),
			RenderableSurfaceManager::GetInstance()->GetDepthSurface(handles[1]),
			RenderableSurfaceManager::GetInstance()->GetDepthSurface(handles[2]),
			RenderableSurfaceManager::GetInstance()->GetDepthSurface(handles[3]),
			RenderableSurfaceManager::GetInstance()->GetDepthSurface(handles[4]),
			RenderableSurfaceManager::GetInstance()->GetDepthSurface(handles[5])
	};
}

void RenderContext::ReleaseDepthSurfacesForAllLights()
{
	// TODO
}
