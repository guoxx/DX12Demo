#include "pch.h"
#include "Renderer.h"

#include "Camera.h"
#include "Scene.h"
#include "Model.h"

#include "Lights/PointLight.h"
#include "Lights/DirectionalLight.h"

#include "Filters/Filter2D.h"
#include "Filters/ToneMapFilter2D.h"
#include "Filters/PointLightFilter2D.h"
#include "Filters/DirectionalLightFilter2D.h"

#include "Pass/LightCullingPass.h"
#include "Pass/TiledShadingPass.h"


Renderer::Renderer(GFX_HWND hwnd, int32_t width, int32_t height)
	: m_Width{ width}
	, m_Height{ height }
{
	DX12Device* pDevice = DX12GraphicManager::GetInstance()->GetDevice();

	m_SwapChain = std::make_shared<DX12SwapChain>(pDevice, hwnd, width, height, GFX_FORMAT_R8G8B8A8_UNORM_SRGB_SWAPCHAIN);

	m_SceneGBuffer0 = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R8G8B8A8_UNORM, width, height));
	m_SceneGBuffer1 = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R8G8B8A8_UNORM, width, height));
	m_SceneGBuffer2 = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R8G8B8A8_UNORM, width, height));
	m_SceneDepthSurface = RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(RenderableSurfaceDesc(GFX_FORMAT_D32_FLOAT, width, height));

	m_LightingSurface = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_HDR, width, height));

	m_IdentityFilter2D = std::make_shared<Filter2D>(pDevice);
	m_ToneMapFilter2D = std::make_shared<ToneMapFilter2D>(pDevice);

	m_PointLightFilter2D = std::make_shared<PointLightFilter2D>(pDevice);
	m_DirLightFilter2D = std::make_shared<DirectionalLightFilter2D>(pDevice);

	uint32_t numTileX = (m_Width + LIGHT_CULLING_NUM_THREADS_XY - 1) / LIGHT_CULLING_NUM_THREADS_XY;
	uint32_t numTileY = (m_Height + LIGHT_CULLING_NUM_THREADS_XY - 1) / LIGHT_CULLING_NUM_THREADS_XY;
	m_AllPointLights = std::make_shared<DX12StructuredBuffer>(pDevice,
		sizeof(HLSL::PointLight) * MAX_POINT_LIGHTS_PER_FRAME, 0, sizeof(HLSL::PointLight),
		DX12GpuResourceUsage_CpuWrite_GpuRead);
	m_AllDirectionalLights = std::make_shared<DX12StructuredBuffer>(pDevice,
		sizeof(HLSL::DirectionalLight) * MAX_DIRECTIONAL_LIGHTS_PER_FRAME, 0, sizeof(HLSL::DirectionalLight),
		DX12GpuResourceUsage_CpuWrite_GpuRead);
	m_VisiblePointLights = std::make_shared<DX12StructuredBuffer>(pDevice,
		sizeof(HLSL::LightNode) * numTileX * numTileY * MAX_LIGHT_NODES_PER_TILE, 0, sizeof(HLSL::LightNode),
		DX12GpuResourceUsage_GpuReadWrite);

	m_LightCullingPass = std::make_shared<LightCullingPass>(pDevice);
	m_TiledShadingPass = std::make_shared<TiledShadingPass>(pDevice);

	m_TiledShading = true;
	m_ToneMapEnabled = true;
	m_ToneMapExposure = 4.0f;
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


	if (m_TiledShading)
	{
		{
			pGfxContext->PIXBeginEvent(L"LightCulling");

			int32_t firstTextureId = 0;

			assert(pScene->GetDirectionalLights().size() <= MAX_DIRECTIONAL_LIGHTS_PER_FRAME);
			assert(pScene->GetPointLights().size() <= MAX_POINT_LIGHTS_PER_FRAME);

			// upload directional lights data
			HLSL::DirectionalLight* pDirLightData;
			m_AllDirectionalLights->MapResource(0, (void**)&pDirLightData);
			for (int i = 0; i < pScene->GetDirectionalLights().size(); ++i)
			{
				auto directionalLight = pScene->GetDirectionalLights()[i];

				DirectX::XMFLOAT4 dir = directionalLight->GetDirection();
				pDirLightData[i].m_Direction = DirectX::XMFLOAT3{ dir.x, dir.y, dir.z };
				pDirLightData[i].m_Irradiance = directionalLight->GetIrradiance();

				DirectX::XMMATRIX mLightView;
				DirectX::XMMATRIX mLightProj;
				directionalLight->GetViewAndProjMatrix(m_RenderContext.GetCamera(), &mLightView, &mLightProj);
				DirectX::XMStoreFloat4x4(&pDirLightData[i].m_mViewProj, DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(mLightView, mLightProj)));

				pDirLightData[i].m_ShadowMapTexId = firstTextureId;
				firstTextureId += 1;
			}
			m_AllDirectionalLights->UnmapResource(0);

			// upload point light data
			HLSL::PointLight* pPointLightData;
			m_AllPointLights->MapResource(0, (void**)&pPointLightData);
			for (int i = 0; i < pScene->GetPointLights().size(); ++i)
			{
				auto pointLight = pScene->GetPointLights()[i];

				DirectX::XMStoreFloat4(&pPointLightData[i].m_Position, pointLight->GetTranslation());
				pPointLightData[i].m_RadiusParam = DirectX::XMFLOAT4{ pointLight->GetRadiusStart(), pointLight->GetRadiusEnd(), 0, 0 };
				pPointLightData[i].m_Intensity = pointLight->GetIntensity();
				for (int face = 0; face < 6; ++face)
				{
					DirectX::XMMATRIX mLightView;
					DirectX::XMMATRIX mLightProj;
					pointLight->GetViewAndProjMatrix(nullptr, (PointLight::AXIS)face, DX12PointLightShadowMapSize, &mLightView, &mLightProj);
					DirectX::XMMATRIX mLightViewProj = DirectX::XMMatrixMultiply(mLightView, mLightProj);
					DirectX::XMStoreFloat4x4(&pPointLightData[i].m_mViewProj[face], DirectX::XMMatrixTranspose(mLightViewProj));
				}

				pPointLightData[i].m_FirstShadowMapTexId = firstTextureId;
				firstTextureId += 6;
			}
			m_AllPointLights->UnmapResource(0);

			pGfxContext->ResourceTransitionBarrier(m_VisiblePointLights.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			m_LightCullingPass->Apply(pGfxContext, &m_RenderContext, pScene);
			pGfxContext->SetComputeRootStructuredBuffer(1, m_AllPointLights.get());
			pGfxContext->SetComputeRootRWStructuredBuffer(2, m_VisiblePointLights.get());
			m_LightCullingPass->Exec(pGfxContext);

			pGfxContext->ResourceTransitionBarrier(m_VisiblePointLights.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

			pGfxContext->PIXEndEvent();
		}

		{
			pGfxContext->PIXBeginEvent(L"TiledShading");

			pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer0), D3D12_RESOURCE_STATE_GENERIC_READ);
			pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer1), D3D12_RESOURCE_STATE_GENERIC_READ);
			pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer2), D3D12_RESOURCE_STATE_GENERIC_READ);
			pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetDepthSurface(m_SceneDepthSurface), D3D12_RESOURCE_STATE_GENERIC_READ);
			pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_LightingSurface), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			for (int i = 0; i < pScene->GetDirectionalLights().size(); ++i)
			{
				auto directionalLight = pScene->GetDirectionalLights()[i];
				DX12DepthSurface* pShadowMapForDirLight = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
				pGfxContext->ResourceTransitionBarrier(pShadowMapForDirLight, D3D12_RESOURCE_STATE_GENERIC_READ);
			}
			for (int i = 0; i < pScene->GetPointLights().size(); ++i)
			{
				auto pointLight = pScene->GetPointLights()[i];
				auto pShadowMapsForPointLight = m_RenderContext.AcquireDepthSurfaceForPointLight(pointLight.get());
				for (int j = 0; j < pShadowMapsForPointLight.size(); ++j)
				{
					pGfxContext->ResourceTransitionBarrier(pShadowMapsForPointLight[j], D3D12_RESOURCE_STATE_GENERIC_READ);
				}
			}

			m_TiledShadingPass->Apply(pGfxContext, &m_RenderContext, pScene);
			pGfxContext->SetComputeRootStructuredBuffer(1, m_AllDirectionalLights.get());
			pGfxContext->SetComputeRootStructuredBuffer(2, m_AllPointLights.get());
			pGfxContext->SetComputeRootStructuredBuffer(3, m_VisiblePointLights.get());
			pGfxContext->SetComputeDynamicCbvSrvUav(4, 0, RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer0)->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetComputeDynamicCbvSrvUav(4, 1, RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer1)->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetComputeDynamicCbvSrvUav(4, 2, RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer2)->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetComputeDynamicCbvSrvUav(4, 3, RenderableSurfaceManager::GetInstance()->GetDepthSurface(m_SceneDepthSurface)->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetComputeDynamicCbvSrvUav(4, 4, RenderableSurfaceManager::GetInstance()->GetColorSurface(m_LightingSurface)->GetStagingUAV().GetCpuHandle());
			int firstTextureId = 0;
			for (int i = 0; i < pScene->GetDirectionalLights().size(); ++i)
			{
				auto directionalLight = pScene->GetDirectionalLights()[i];
				DX12DepthSurface* pShadowMapForDirLight = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
				pGfxContext->SetComputeDynamicCbvSrvUav(5, firstTextureId, pShadowMapForDirLight->GetStagingSRV().GetCpuHandle());
				firstTextureId += 1;
			}
			for (int i = 0; i < pScene->GetPointLights().size(); ++i)
			{
				auto pointLight = pScene->GetPointLights()[i];
				auto pShadowMapsForPointLight = m_RenderContext.AcquireDepthSurfaceForPointLight(pointLight.get());
				for (int j = 0; j < pShadowMapsForPointLight.size(); ++j)
				{
					pGfxContext->SetComputeDynamicCbvSrvUav(5, firstTextureId, pShadowMapsForPointLight[j]->GetStagingSRV().GetCpuHandle());
					firstTextureId += 1;
				}
			}
			m_TiledShadingPass->Exec(pGfxContext);

			pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer0), D3D12_RESOURCE_STATE_RENDER_TARGET);
			pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer1), D3D12_RESOURCE_STATE_RENDER_TARGET);
			pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_SceneGBuffer2), D3D12_RESOURCE_STATE_RENDER_TARGET);
			pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetDepthSurface(m_SceneDepthSurface), D3D12_RESOURCE_STATE_DEPTH_WRITE);
			pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_LightingSurface), D3D12_RESOURCE_STATE_RENDER_TARGET);

			for (int i = 0; i < pScene->GetDirectionalLights().size(); ++i)
			{
				auto directionalLight = pScene->GetDirectionalLights()[i];
				DX12DepthSurface* pShadowMapForDirLight = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
				pGfxContext->ResourceTransitionBarrier(pShadowMapForDirLight, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			}
			for (int i = 0; i < pScene->GetPointLights().size(); ++i)
			{
				auto pointLight = pScene->GetPointLights()[i];
				auto pShadowMapsForPointLight = m_RenderContext.AcquireDepthSurfaceForPointLight(pointLight.get());
				for (int j = 0; j < pShadowMapsForPointLight.size(); ++j)
				{
					pGfxContext->ResourceTransitionBarrier(pShadowMapsForPointLight[j], D3D12_RESOURCE_STATE_DEPTH_WRITE);
				}
			}

			pGfxContext->PIXEndEvent();
		}
	}
	else
	{
		pGfxContext->PIXBeginEvent(L"DeferredLighting");

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

	pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_LightingSurface), D3D12_RESOURCE_STATE_GENERIC_READ);
	if (m_ToneMapEnabled)
	{
		m_ToneMapFilter2D->Apply(pGfxContext);
		pGfxContext->SetGraphicsRoot32BitConstants(0, 1, &m_ToneMapExposure, 0);
		pGfxContext->SetGraphicsRootDescriptorTable(1, RenderableSurfaceManager::GetInstance()->GetColorSurface(m_LightingSurface)->GetSRV());
		m_ToneMapFilter2D->Draw(pGfxContext);
	}
	else
	{
		m_IdentityFilter2D->Apply(pGfxContext);
		pGfxContext->SetGraphicsRootDescriptorTable(0, RenderableSurfaceManager::GetInstance()->GetColorSurface(m_LightingSurface)->GetSRV());
		m_IdentityFilter2D->Draw(pGfxContext);
	}
	pGfxContext->ResourceTransitionBarrier(RenderableSurfaceManager::GetInstance()->GetColorSurface(m_LightingSurface), D3D12_RESOURCE_STATE_RENDER_TARGET);
}

