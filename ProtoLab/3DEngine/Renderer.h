#pragma once

#include "../DX12/DX12.h"

#include "RenderContext.h"
#include "RenderableSurfaceManager.h"

class ILight;
class Scene;
class Camera;
class Filter2D;
class ToneMapFilter2D;
class PointLightFilter2D;
class DirectionalLightFilter2D;
class ConvertEVSMFilter2D;
class AntiAliasingFilter2D;
class LightCullingPass;
class TiledShadingPass;

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

    void PostProcess(const Camera* pCamera, Scene* pScene);

	void ResolveToSwapChain();

	void RenderDebugMenu();

	int32_t m_Width;
	int32_t m_Height;
    int32_t m_FrameIdx;

	RenderContext m_RenderContext;	

	RenderableSurfaceHandle<DX12ColorSurface> m_SceneGBuffer0;
	RenderableSurfaceHandle<DX12ColorSurface> m_SceneGBuffer1;
	RenderableSurfaceHandle<DX12ColorSurface> m_SceneGBuffer2;
	RenderableSurfaceHandle<DX12DepthSurface> m_SceneDepthSurface;

	RenderableSurfaceHandle<DX12ColorSurface> m_LightingSurface;
	RenderableSurfaceHandle<DX12ColorSurface> m_PostProcessSurface;
	RenderableSurfaceHandle<DX12ColorSurface> m_HistoryLightingSurface;

	std::shared_ptr<DX12SwapChain> m_SwapChain;

	std::shared_ptr<Filter2D> m_IdentityFilter2D;
	std::shared_ptr<ToneMapFilter2D> m_ToneMapFilter2D;
	std::shared_ptr<PointLightFilter2D> m_PointLightFilter2D;
	std::shared_ptr<DirectionalLightFilter2D> m_DirLightFilter2D;
	std::shared_ptr<ConvertEVSMFilter2D> m_ConvertEVSMFilter2D;
	std::shared_ptr<AntiAliasingFilter2D> m_AntiAliasingFilter2D;

	std::shared_ptr<LightCullingPass> m_LightCullingPass;
	std::shared_ptr<TiledShadingPass> m_TiledShadingPass;
	std::shared_ptr<DX12StructuredBuffer> m_AllPointLights;
	std::shared_ptr<DX12StructuredBuffer> m_AllDirectionalLights;
	std::shared_ptr<DX12StructuredBuffer> m_VisiblePointLights;
};

