#include "pch.h"
#include "Renderer.h"

#include "Camera.h"
#include "Scene.h"
#include "Model.h"

#include "Lights/PointLight.h"
#include "Lights/DirectionalLight.h"

#include "Filters/Filter2D.h"
#include "Filters/PointLightFilter2D.h"
#include "Filters/DirectionalLightFilter2D.h"

#include "Pass/LightCullingPass.h"


Renderer::Renderer(GFX_HWND hwnd, int32_t width, int32_t height)
	: m_Width{ width}
	, m_Height{ height }
{
	DX12Device* pDevice = DX12GraphicManager::GetInstance()->GetDevice();

	m_SwapChain = std::make_shared<DX12SwapChain>(pDevice, hwnd, width, height, GFX_FORMAT_R8G8B8A8_UNORM_SWAPCHAIN);

	m_SceneGBuffer0 = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R8G8B8A8_UNORM, width, height));
	m_SceneGBuffer1 = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R8G8B8A8_UNORM, width, height));
	m_SceneGBuffer2 = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R8G8B8A8_UNORM, width, height));
	m_SceneDepthSurface = RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(RenderableSurfaceDesc(GFX_FORMAT_D32_FLOAT, width, height));

	m_LightingSurface = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R32G32B32A32_FLOAT, width, height));

	m_IdentityFilter2D = std::make_shared<Filter2D>(pDevice);

	m_PointLightFilter2D = std::make_shared<PointLightFilter2D>(pDevice);
	m_DirLightFilter2D = std::make_shared<DirectionalLightFilter2D>(pDevice);

	uint32_t numTileX = (m_Width + LIGHT_CULLING_NUM_THREADS_XY - 1) / LIGHT_CULLING_NUM_THREADS_XY;
	uint32_t numTileY = (m_Height + LIGHT_CULLING_NUM_THREADS_XY - 1) / LIGHT_CULLING_NUM_THREADS_XY;
	m_AllPointLightForCulling = std::make_shared<DX12StructuredBuffer>(pDevice,
		sizeof(ShapeSphere) * MAX_POINT_LIGHTS_PER_FRAME, 0, sizeof(ShapeSphere),
		DX12GpuResourceUsage_CpuWrite_GpuRead);
	m_VisiblePointLights = std::make_shared<DX12StructuredBuffer>(pDevice,
		sizeof(LightNode) * numTileX * numTileY * MAX_LIGHT_NODES_PER_TILE, 0, sizeof(LightNode),
		DX12GpuResourceUsage_GpuReadWrite);

	m_LightCullingPass = std::make_shared<LightCullingPass>(pDevice);
}

Renderer::~Renderer()
{
}

void Renderer::Render(const Camera* pCamera, Scene* pScene)
{
	m_RenderContext.SetCamera(pCamera);
	m_RenderContext.SetScreenSize(m_Width, m_Height);

	RenderShadowMaps(pCamera, pScene);
	RenderGBuffer(pCamera, pScene);
	DeferredLighting(pCamera, pScene);
	ResolveToSwapChain();
}

void Renderer::Flip()
{
	m_SwapChain->Flip();
	DX12GraphicManager::GetInstance()->Flip();
}

void Renderer::RenderShadowMaps(const Camera* pCamera, Scene * pScene)
{
	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	for (auto directionalLight : pScene->GetDirectionalLights())
	{
		m_RenderContext.SetShadingCfg(ShadingConfiguration_DepthOnly);

		pGfxContext->PIXBeginEvent(L"ShadowMap - DirectionalLight");

		DX12DepthSurface* pDepthSurface = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());

		pGfxContext->ClearDepthTarget(pDepthSurface, 1.0f);
		pGfxContext->SetRenderTargets(0, nullptr, pDepthSurface);
		pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pGfxContext->SetViewport(0, 0, DX12DirectionalLightShadowMapSize, DX12DirectionalLightShadowMapSize);

		m_RenderContext.SetModelMatrix(DirectX::XMMatrixIdentity());
		DirectX::XMMATRIX mLightView;
		DirectX::XMMATRIX mLightProj;
		directionalLight->GetViewAndProjMatrix(pCamera, &mLightView, &mLightProj);
		m_RenderContext.SetViewMatrix(mLightView);
		m_RenderContext.SetProjMatrix(mLightProj);

		for (auto model : pScene->GetModels())
		{
			model->DrawPrimitives(&m_RenderContext, pGfxContext);
		}

		pGfxContext->PIXEndEvent();
	}

	for (auto pointLight : pScene->GetPointLights())
	{
		m_RenderContext.SetShadingCfg(ShadingConfiguration_DepthOnly);

		pGfxContext->PIXBeginEvent(L"ShadowMap - PointLight");

		auto pDepthSurfaces = m_RenderContext.AcquireDepthSurfaceForPointLight(pointLight.get());

		for (int i = 0; i < 6; ++i)
		{
			wchar_t* axisNames[] = { L"POSITIVE_X", L"NEGATIVE_X", L"POSITIVE_Y", L"NEGATIVE_Y", L"POSITIVE_Z", L"NEGATIVE_Z" };
			pGfxContext->PIXBeginEvent(axisNames[i]);

			pGfxContext->ClearDepthTarget(pDepthSurfaces[i], 1.0f);
			pGfxContext->SetRenderTargets(0, nullptr, pDepthSurfaces[i]);
			pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			pGfxContext->SetViewport(0, 0, DX12PointLightShadowMapSize, DX12PointLightShadowMapSize);

			m_RenderContext.SetModelMatrix(DirectX::XMMatrixIdentity());
			DirectX::XMMATRIX mLightView;
			DirectX::XMMATRIX mLightProj;
			pointLight->GetViewAndProjMatrix(pCamera, (PointLight::AXIS)i, DX12PointLightShadowMapSize, &mLightView, &mLightProj);
			m_RenderContext.SetViewMatrix(mLightView);
			m_RenderContext.SetProjMatrix(mLightProj);

			for (auto model : pScene->GetModels())
			{
				model->DrawPrimitives(&m_RenderContext, pGfxContext);
			}

			pGfxContext->PIXEndEvent();
		}
		pGfxContext->PIXEndEvent();
	}
}

void Renderer::DeferredLighting(const Camera* pCamera, Scene* pScene)
{
	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	pGfxContext->PIXBeginEvent(L"DeferredLighting");

	{
		pGfxContext->PIXBeginEvent(L"LightCulling");

		ShapeSphere* pData;
		m_AllPointLightForCulling->MapResource(0, (void**)&pData);

		int i = 0;
		for (auto pointLight : pScene->GetPointLights())
		{
			DirectX::XMVECTOR pos = pointLight->GetTranslation();
			pData[i].m_Position = DirectX::XMFLOAT3{ DirectX::XMVectorGetX(pos), DirectX::XMVectorGetY(pos), DirectX::XMVectorGetZ(pos) };
			pData[i].m_Radius = pointLight->GetRadiusEnd();

			i += 1;
		}

		m_AllPointLightForCulling->UnmapResource(0);

		m_LightCullingPass->Apply(pGfxContext, &m_RenderContext, pScene);
		pGfxContext->SetComputeDynamicCbvSrvUav(1, 0, m_AllPointLightForCulling->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetComputeDynamicCbvSrvUav(1, 1, m_VisiblePointLights->GetStagingUAV().GetCpuHandle());
		m_LightCullingPass->Exec(pGfxContext);

		pGfxContext->PIXEndEvent();
	}

    pGfxContext->ClearRenderTarget(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_LightingSurface), 0, 0, 0, 0);

	// setup color and depth buffers
	DX12ColorSurface* pColorSurfaces[] = { RenderableSurfaceManager::GetInstance()->GetColorSurface(m_LightingSurface) };
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, nullptr);

	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

	m_RenderContext.SetModelMatrix(DirectX::XMMatrixIdentity());
	m_RenderContext.SetViewMatrix(pCamera->GetViewMatrix());
	m_RenderContext.SetProjMatrix(pCamera->GetProjectionMatrix());

	pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer0), D3D12_RESOURCE_STATE_GENERIC_READ);
	pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer1), D3D12_RESOURCE_STATE_GENERIC_READ);
	pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer2), D3D12_RESOURCE_STATE_GENERIC_READ);
	pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetDepthSurface(m_SceneDepthSurface), D3D12_RESOURCE_STATE_GENERIC_READ);

	for (auto directionalLight : pScene->GetDirectionalLights())
	{
		DX12DepthSurface* pShadowMapForDirLight = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
		pGfxContext->ResourceTransitionBarrier(pShadowMapForDirLight, D3D12_RESOURCE_STATE_GENERIC_READ);

		m_DirLightFilter2D->Apply(pGfxContext, &m_RenderContext, directionalLight.get());

		pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 0, RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer0)->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 1, RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer1)->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 2, RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer2)->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 3, RenderableSurfaceManager::GetInstance()->GetDepthSurface(m_SceneDepthSurface)->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 4, pShadowMapForDirLight->GetStagingSRV().GetCpuHandle());

		m_DirLightFilter2D->Draw(pGfxContext);

		pGfxContext->ResourceTransitionBarrier(pShadowMapForDirLight, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	for (auto pointLight : pScene->GetPointLights())
	{
		auto pShadowMapsForPointLight = m_RenderContext.AcquireDepthSurfaceForPointLight(pointLight.get());

		for (int i = 0; i < pShadowMapsForPointLight.size(); ++i)
		{
			pGfxContext->ResourceTransitionBarrier(pShadowMapsForPointLight[i], D3D12_RESOURCE_STATE_GENERIC_READ);
		}

		m_PointLightFilter2D->Apply(pGfxContext, &m_RenderContext, pointLight.get());

		pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 0, RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer0)->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 1, RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer1)->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 2, RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer2)->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 3, RenderableSurfaceManager::GetInstance()->GetDepthSurface(m_SceneDepthSurface)->GetStagingSRV().GetCpuHandle());
		for (int i = 0; i < pShadowMapsForPointLight.size(); ++i)
		{
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 4 + i, pShadowMapsForPointLight[i]->GetStagingSRV().GetCpuHandle());
		}

		m_DirLightFilter2D->Draw(pGfxContext);

		for (int i = 0; i < pShadowMapsForPointLight.size(); ++i)
		{
			pGfxContext->ResourceTransitionBarrier(pShadowMapsForPointLight[i], D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}
	}

	pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer0), D3D12_RESOURCE_STATE_RENDER_TARGET);
	pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer1), D3D12_RESOURCE_STATE_RENDER_TARGET);
	pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer2), D3D12_RESOURCE_STATE_RENDER_TARGET);
	pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetDepthSurface(m_SceneDepthSurface), D3D12_RESOURCE_STATE_DEPTH_WRITE);

	pGfxContext->PIXEndEvent();
}

void Renderer::RenderGBuffer(const Camera* pCamera, Scene* pScene)
{
	m_RenderContext.SetShadingCfg(ShadingConfiguration_GBuffer);

	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	pGfxContext->PIXBeginEvent(L"G-Buffer");

    // Clear the views.
    pGfxContext->ClearRenderTarget(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer0), 0, 0, 0, 0);
    pGfxContext->ClearRenderTarget(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer1), 0, 0, 0, 0);
    pGfxContext->ClearRenderTarget(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer2), 0, 0, 0, 0);
	pGfxContext->ClearDepthTarget(RenderableSurfaceManager::GetInstance()->GetDepthSurface(m_SceneDepthSurface), 1.0f);

	// setup color and depth buffers
	DX12ColorSurface* pColorSurfaces[] = {
		RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer0),
		RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer1),
		RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer2)
	};
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, RenderableSurfaceManager::GetInstance()->GetDepthSurface(m_SceneDepthSurface));

	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

	m_RenderContext.SetModelMatrix(DirectX::XMMatrixIdentity());
	m_RenderContext.SetViewMatrix(pCamera->GetViewMatrix());
	m_RenderContext.SetProjMatrix(pCamera->GetProjectionMatrix());

	for (auto model : pScene->GetModels())
	{
		model->DrawPrimitives(&m_RenderContext, pGfxContext);
	}

	pGfxContext->PIXEndEvent();
}

void Renderer::ResolveToSwapChain()
{
	m_SwapChain->Begin();

	DX12SwapChainContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

    // Clear the views.
	DX12ColorSurface* pColorSurface = m_SwapChain->GetBackBuffer();
	DX12ColorSurface* pColorSurfaces[] = { pColorSurface };
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, nullptr);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

	m_IdentityFilter2D->Apply(pGfxContext);
	pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_LightingSurface), D3D12_RESOURCE_STATE_GENERIC_READ);
	pGfxContext->SetGraphicsRootDescriptorTable(0, RenderableSurfaceManager::GetInstance()->GetColorSurface(m_LightingSurface)->GetSRV());
	m_IdentityFilter2D->Draw(pGfxContext);
	pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_LightingSurface), D3D12_RESOURCE_STATE_RENDER_TARGET);
}

