#pragma once

#include "../DX12/DX12.h"

#include "RenderContext.h"

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
	void DeferredLighting(const Camera* pCamera, Scene* pScene);

	void RenderGBuffer(const Camera* pCamera, Scene* pScene);

	void ResolveToSwapChain();

	int32_t m_Width;
	int32_t m_Height;

	RenderContext m_RenderContext;	

	std::shared_ptr<DX12ColorSurface> m_SceneGBuffer0;
	std::shared_ptr<DX12ColorSurface> m_SceneGBuffer1;
	std::shared_ptr<DX12ColorSurface> m_SceneGBuffer2;
	std::shared_ptr<DX12DepthSurface> m_SceneDepthSurface;

	std::shared_ptr<DX12ColorSurface> m_LightingSurface;

	std::shared_ptr<DX12SwapChain> m_SwapChain;

	std::shared_ptr<Filter2D> m_IdentityFilter2D;
	std::shared_ptr<DirectionalLightFilter2D> m_DirLightFilter2D;
};

