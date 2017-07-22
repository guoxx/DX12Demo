#pragma once

#include "../DX12/DX12.h"

#include "SurfaceSet.h"
#include "RenderContext.h"
#include "RenderableSurfaceManager.h"
#include "Pass/ScreenSpaceShadowsPass.h"
#include "Pass/PointLightShadingPass.h"
#include "Pass/DirectionalLightShadingPass.h"
#include "Pass/ToneMappingPass.h"
#include "Pass/TemporalAAPass.h"

class ILight;
class Scene;
class Camera;
class LightCullingPass;
class TiledShadingPass;
class ComputeProcessing;

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

    void SkyDome(const Camera* pCamera, Scene* pScene);

	void RenderGBuffer(const Camera* pCamera, Scene* pScene);

    void PostProcess(const Camera* pCamera, Scene* pScene);

	void ResolveToSwapChain();

	void RenderDebugMenu();

    void AAFilter();

    void CalcAvgLuminance(DX12ColorSurface* surf);

    void ToneMap();

    void ScreenSpaceShadows(const Camera* pCamera, Scene* pScene);

    int32_t m_Width;
	int32_t m_Height;
    int32_t m_FrameIdx;

	RenderContext m_RenderContext;	
    GBufferSurfaceSet m_GBuffer;
    PostProcessSurfaceSet m_PostProcessSurfaces;

	std::shared_ptr<DX12SwapChain> m_SwapChain;

	std::shared_ptr<LightCullingPass> m_LightCullingPass;
	std::shared_ptr<TiledShadingPass> m_TiledShadingPass;
	std::shared_ptr<DX12StructuredBuffer> m_AllPointLights;
	std::shared_ptr<DX12StructuredBuffer> m_AllDirectionalLights;
	std::shared_ptr<DX12StructuredBuffer> m_VisiblePointLights;

    std::shared_ptr<ScreenSpaceShadowsPass> m_ScreenSpaceShadowPass;
    std::shared_ptr<PointLightShadingPass> m_PointLightShadingPass;
    std::shared_ptr<DirectionalLightShadingPass> m_DirectionalLightShadingPass;

    std::shared_ptr<TemporalAAPass> m_TemporalAA;
    std::shared_ptr<ToneMappingPass> m_ToneMap;

    std::shared_ptr<ImageProcessing> m_ResolveToSwapChain;

    std::shared_ptr<ImageProcessing> m_EVSM;

    std::shared_ptr<ImageProcessing> m_RenderSky;

    std::shared_ptr<ComputeProcessing> m_ReduceLuminanceInitial;
    std::shared_ptr<ComputeProcessing> m_ReduceLuminance;
    std::shared_ptr<ComputeProcessing> m_ReduceLuminanceFinal;
};

