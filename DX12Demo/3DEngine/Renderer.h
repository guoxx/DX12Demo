#pragma once

#include "../DX12/DX12.h"

#include "RenderContext.h"
#include "RenderableSurfaceManager.h"

class ILight;
class Scene;
class Camera;
class Filter2D;
class DirectionalLightFilter2D;

class Renderer
{
public:
	Renderer(GFX_HWND hwnd, int32_t width, int32_t height);
	~Renderer();

	void Render(const Camera* pCamera, Scene* pScene);

	void Flip();

private:
	void RenderShadowMaps(const Camera* pCamera, Scene* pScene);

	void DeferredLighting(const Camera* pCamera, Scene* pScene);

	void RenderGBuffer(const Camera* pCamera, Scene* pScene);

	void ResolveToSwapChain();

	int32_t m_Width;
	int32_t m_Height;

	RenderContext m_RenderContext;	

	RenderableSurfaceHandle m_SceneGBuffer0;
	RenderableSurfaceHandle m_SceneGBuffer1;
	RenderableSurfaceHandle m_SceneGBuffer2;
	RenderableSurfaceHandle m_SceneDepthSurface;

	RenderableSurfaceHandle m_LightingSurface;

	std::shared_ptr<DX12SwapChain> m_SwapChain;

	std::shared_ptr<Filter2D> m_IdentityFilter2D;
	std::shared_ptr<DirectionalLightFilter2D> m_DirLightFilter2D;
};

