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
#include "Filters/ConvertEVSMFilter2D.h"
#include "Filters/AntiAliasingFilter2D.h"
#include "Filters/ResolveToSwapChainFilter2D.h"
#include "Filters/ComputeProcessing.h"

#include "Pass/LightCullingPass.h"
#include "Pass/TiledShadingPass.h"

#include "Shaders/CompiledShaders/LuminanceReductionInitial.h"
#include "Shaders/CompiledShaders/LuminanceReduction.h"
#include "Shaders/CompiledShaders/LuminanceReductionFinal.h"


Renderer::Renderer(GFX_HWND hwnd, int32_t width, int32_t height)
	: m_Width{ width}
	, m_Height{ height }
    , m_FrameIdx{ 0 }
{
	DX12Device* pDevice = DX12GraphicsManager::GetInstance()->GetDevice();

	m_SwapChain = std::make_shared<DX12SwapChain>(pDevice, hwnd, width, height, GFX_FORMAT_SWAPCHAIN);

	m_SceneGBuffer0 = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R8G8B8A8_UNORM, width, height));
	m_SceneGBuffer1 = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R8G8B8A8_UNORM, width, height));
	m_SceneGBuffer2 = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R8G8B8A8_UNORM, width, height));
	m_SceneGBuffer3 = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R16G16B16A16_FLOAT, width, height));
	m_SceneDepthSurface = RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(RenderableSurfaceDesc(GFX_FORMAT_D32_FLOAT, width, height));

	m_LightingSurface = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_HDR, width, height));
	m_HistoryLightingSurface = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_HDR, width, height));
	m_PostProcessSurface = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_HDR, width, height));

	m_LDRSurface = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_LDR, width, height));

	m_IdentityFilter2D = std::make_shared<Filter2D>(pDevice);
    m_ResolveToSwapChainFilter2D = std::make_shared<ResolveToSwapChainFilter2D>(pDevice);
	m_ToneMapFilter2D = std::make_shared<ToneMapFilter2D>(pDevice);

	m_PointLightFilter2D = std::make_shared<PointLightFilter2D>(pDevice);
	m_DirLightFilter2D = std::make_shared<DirectionalLightFilter2D>(pDevice);

	m_ConvertEVSMFilter2D = std::make_shared<ConvertEVSMFilter2D>(pDevice);

    m_AntiAliasingFilter2D = std::make_shared<AntiAliasingFilter2D>(pDevice);

    uint32_t lumReductionWidth = m_Width;
    uint32_t lumReductionHeight = m_Height;
    while (true)
    {
        lumReductionWidth = DX::DivideByMultiple(lumReductionWidth, LUMINANCE_REDUCTION_THREAD_GROUP_SIZE);
        lumReductionHeight = DX::DivideByMultiple(lumReductionHeight, LUMINANCE_REDUCTION_THREAD_GROUP_SIZE);

	    auto surf = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R32_FLOAT, lumReductionWidth, lumReductionHeight));
        m_LuminanceSurfaces.push_back(surf);

        if (lumReductionWidth == 1 && lumReductionHeight == 1)
        {
            break;
        }
    }
    assert(m_LuminanceSurfaces.size() >= 2);

	uint32_t numTileX = DX::DivideByMultiple(m_Width, LIGHT_CULLING_NUM_THREADS_XY);
	uint32_t numTileY = DX::DivideByMultiple(m_Height, LIGHT_CULLING_NUM_THREADS_XY);
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

    m_ReduceLuminanceInitial = std::make_shared<ComputeProcessing>(pDevice, g_LuminanceReductionInitial_CS, sizeof(g_LuminanceReductionInitial_CS));
    m_ReduceLuminance = std::make_shared<ComputeProcessing>(pDevice, g_LuminanceReduction_CS, sizeof(g_LuminanceReduction_CS));
    m_ReduceLuminanceFinal = std::make_shared<ComputeProcessing>(pDevice, g_LuminanceReductionFinal_CS, sizeof(g_LuminanceReductionFinal_CS));
}

Renderer::~Renderer()
{
}

void Renderer::Render(const Camera* pCamera, Scene* pScene)
{
	m_RenderContext.SetCamera(pCamera);
	m_RenderContext.SetScreenSize(m_Width, m_Height);
    if (m_FrameIdx & 0x01)
    {
        m_RenderContext.SetJitter(-0.25, 0.25);
    }
    else
    {
        m_RenderContext.SetJitter(0.25, -0.25);
    }

	RenderShadowMaps(pCamera, pScene);
	RenderGBuffer(pCamera, pScene);
	DeferredLighting(pCamera, pScene);
    PostProcess(pCamera, pScene);
	ResolveToSwapChain();
	RenderDebugMenu();
}

void Renderer::Flip()
{
	m_SwapChain->Flip();
	DX12GraphicsManager::GetInstance()->Flip();

    ++m_FrameIdx;
}

void Renderer::RenderShadowMaps(const Camera* pCamera, Scene * pScene)
{
	DX12ScopedGraphicsContext pGfxContext;

	if (g_RSMEnabled)
	{
		m_RenderContext.SetShadingCfg(ShadingConfiguration_RSM);
	}
	else
	{
		m_RenderContext.SetShadingCfg(ShadingConfiguration_DepthOnly);
	}

	for (auto directionalLight : pScene->GetDirectionalLights())
	{
		pGfxContext->PIXBeginEvent(L"ShadowMap - DirectionalLight");

        directionalLight->PrepareForShadowPass(pCamera);

		if (g_RSMEnabled)
		{
			m_RenderContext.SetCurrentLightForRSM(directionalLight.get());

			DX12ColorSurface* pIntensitySurface = m_RenderContext.AcquireRSMRadiantIntensitySurfaceForDirectionalLight(directionalLight.get());
			DX12ColorSurface* pNormalSurface = m_RenderContext.AcquireRSMNormalSurfaceForDirectionalLight(directionalLight.get());
			DX12DepthSurface* pDepthSurface = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());

			pGfxContext->ResourceTransitionBarrier(pDepthSurface, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			pGfxContext->ClearRenderTarget(pIntensitySurface, 0, 0, 0, 0);
			pGfxContext->ClearRenderTarget(pNormalSurface, 1, 0, 0, 0);
			pGfxContext->ClearDepthTarget(pDepthSurface, 1.0f);

			DX12ColorSurface* pSurfaces[] = { pIntensitySurface, pNormalSurface };
			pGfxContext->SetRenderTargets(_countof(pSurfaces), pSurfaces, pDepthSurface);
		}
		else
		{
			DX12DepthSurface* pDepthSurface = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
			pGfxContext->ResourceTransitionBarrier(pDepthSurface, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			pGfxContext->ClearDepthTarget(pDepthSurface, 1.0f);
			pGfxContext->SetRenderTargets(0, nullptr, pDepthSurface);
		}

        for (int32_t cascadeIdx = 0; cascadeIdx < DirectionalLight::NUM_CASCADED_SHADOW_MAP; ++cascadeIdx)
        {
            static_assert(DirectionalLight::NUM_CASCADED_SHADOW_MAP == 4, "");
            static const wchar_t* markers[4] = {L"Cascade - 0", L"Cascade - 1", L"Cascade - 2", L"Cascade - 3"};
            pGfxContext->PIXBeginEvent(markers[cascadeIdx]);

            int32_t tileX = cascadeIdx & 0x01;
            int32_t tileY = (cascadeIdx & 0x02) > 1;

            pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            pGfxContext->SetViewport(tileX * DX12DirectionalLightShadowMapSize / 2,
                                     tileY * DX12DirectionalLightShadowMapSize / 2,
                                     DX12DirectionalLightShadowMapSize / 2,
                                     DX12DirectionalLightShadowMapSize / 2);

            m_RenderContext.SetModelMatrix(DirectX::XMMatrixIdentity());
            DirectX::XMMATRIX mLightView;
            DirectX::XMMATRIX mLightProj;
            directionalLight->GetViewAndProjMatrix(cascadeIdx, &mLightView, &mLightProj);
            m_RenderContext.SetViewMatrix(mLightView);
            m_RenderContext.SetProjMatrix(mLightProj);

            for (auto model : pScene->GetModels())
            {
                model->DrawPrimitives(&m_RenderContext, pGfxContext.Get());
            }

            pGfxContext->PIXEndEvent();
        }

		if (g_EVSMEnabled)
		{
			DX12DepthSurface* pDepthSurface = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
			DX12ColorSurface* pEVSMSurface = m_RenderContext.AcquireEVSMSurfaceForDirectionalLight(directionalLight.get());

			DX12ColorSurface* pColorSurfaces[] = { pEVSMSurface };
			pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, nullptr);

		    pGfxContext->SetViewport(0, 0, DX12DirectionalLightShadowMapSize, DX12DirectionalLightShadowMapSize);

			pGfxContext->ResourceTransitionBarrier(pDepthSurface, D3D12_RESOURCE_STATE_GENERIC_READ);

			m_ConvertEVSMFilter2D->Apply(pGfxContext.Get());
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 0, pDepthSurface->GetStagingSRV().GetCpuHandle());
			m_ConvertEVSMFilter2D->Draw(pGfxContext.Get());

			pGfxContext->ResourceTransitionBarrier(pDepthSurface, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}

		pGfxContext->PIXEndEvent();
	}

	for (auto pointLight : pScene->GetPointLights())
	{
		pGfxContext->PIXBeginEvent(L"ShadowMap - PointLight");

        pointLight->PrepareForShadowPass(pCamera, DX12PointLightShadowMapSize);

		for (int i = 0; i < 6; ++i)
		{
			wchar_t* axisNames[] = { L"POSITIVE_X", L"NEGATIVE_X", L"POSITIVE_Y", L"NEGATIVE_Y", L"POSITIVE_Z", L"NEGATIVE_Z" };
			pGfxContext->PIXBeginEvent(axisNames[i]);

			if (g_RSMEnabled)
			{
				m_RenderContext.SetCurrentLightForRSM(pointLight.get());

				auto pIntensitySurfaces = m_RenderContext.AcquireRSMRadiantIntensitySurfaceForPointLight(pointLight.get());
				auto pNormalSurfaces = m_RenderContext.AcquireRSMNormalSurfaceForPointLight(pointLight.get());
				auto pDepthSurfaces = m_RenderContext.AcquireDepthSurfaceForPointLight(pointLight.get());

				pGfxContext->ClearRenderTarget(pIntensitySurfaces[i], 0, 0, 0, 0);
				pGfxContext->ClearRenderTarget(pNormalSurfaces[i], 1, 0, 0, 0);
				pGfxContext->ClearDepthTarget(pDepthSurfaces[i], 1.0f);

				DX12ColorSurface* pSurfaces[] = { pIntensitySurfaces[i], pNormalSurfaces[i] };
				pGfxContext->SetRenderTargets(_countof(pSurfaces), pSurfaces, pDepthSurfaces[i]);
			}
			else
			{
				auto pDepthSurfaces = m_RenderContext.AcquireDepthSurfaceForPointLight(pointLight.get());
				pGfxContext->ClearDepthTarget(pDepthSurfaces[i], 1.0f);
				pGfxContext->SetRenderTargets(0, nullptr, pDepthSurfaces[i]);
			}

			pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			pGfxContext->SetViewport(0, 0, DX12PointLightShadowMapSize, DX12PointLightShadowMapSize);

			m_RenderContext.SetModelMatrix(DirectX::XMMatrixIdentity());
			DirectX::XMMATRIX mLightView;
			DirectX::XMMATRIX mLightProj;
			pointLight->GetViewAndProjMatrix((PointLight::AXIS)i, &mLightView, &mLightProj);
			m_RenderContext.SetViewMatrix(mLightView);
			m_RenderContext.SetProjMatrix(mLightProj);

			for (auto model : pScene->GetModels())
			{
				model->DrawPrimitives(&m_RenderContext, pGfxContext.Get());
			}

			pGfxContext->PIXEndEvent();
		}
		pGfxContext->PIXEndEvent();
	}
}

void Renderer::DeferredLighting(const Camera* pCamera, Scene* pScene)
{
	DX12ScopedGraphicsContext pGfxContext;


	if (g_TiledShading)
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

				DirectX::XMFLOAT4 direction = directionalLight->GetDirection();
				DirectX::XMFLOAT4 irradiance = directionalLight->GetIrradiance();
				pDirLightData[i].m_Direction = DirectX::XMFLOAT3{ direction.x, direction.y, direction.z };
				pDirLightData[i].m_Irradiance = DirectX::XMFLOAT3{ irradiance.x, irradiance.y, irradiance.z };

                for (int32_t cascadeIdx = 0; cascadeIdx < DirectionalLight::NUM_CASCADED_SHADOW_MAP; ++cascadeIdx)
                {
                    DirectX::XMMATRIX mLightView;
                    DirectX::XMMATRIX mLightProj;
                    directionalLight->GetViewAndProjMatrix(cascadeIdx, &mLightView, &mLightProj);
                    DirectX::XMMATRIX mLightViewProj = DirectX::XMMatrixMultiply(mLightView, mLightProj);
                    DirectX::XMStoreFloat4x4(&pDirLightData[i].m_mViewProj[cascadeIdx], DirectX::XMMatrixTranspose(mLightViewProj));
                    DirectX::XMMATRIX mInvLightViewProj = DirectX::XMMatrixInverse(nullptr, mLightViewProj);
                    DirectX::XMStoreFloat4x4(&pDirLightData[i].m_mInvViewProj[cascadeIdx], DirectX::XMMatrixTranspose(mInvLightViewProj));
                }

				pDirLightData[i].m_ShadowMapTexId = firstTextureId;
				pDirLightData[i].m_RSMIntensityTexId = firstTextureId + 1;
				pDirLightData[i].m_RSMNormalTexId = firstTextureId + 2;
				pDirLightData[i].m_EVSMTexId = firstTextureId + 3;
				firstTextureId += 4;
			}
			m_AllDirectionalLights->UnmapResource(0);

			// upload point light data
			HLSL::PointLight* pPointLightData;
			m_AllPointLights->MapResource(0, (void**)&pPointLightData);
			for (int i = 0; i < pScene->GetPointLights().size(); ++i)
			{
				auto pointLight = pScene->GetPointLights()[i];

				DirectX::XMStoreFloat4(&pPointLightData[i].m_PositionAndRadius, pointLight->GetTranslation());
                pPointLightData[i].m_PositionAndRadius.w = pointLight->GetRadius();
                DirectX::XMStoreFloat4(&pPointLightData[i].m_RadiantPower, pointLight->GetRadiantPower());
				for (int face = 0; face < 6; ++face)
				{
					DirectX::XMMATRIX mLightView;
					DirectX::XMMATRIX mLightProj;
					pointLight->GetViewAndProjMatrix((PointLight::AXIS)face, &mLightView, &mLightProj);
					DirectX::XMMATRIX mLightViewProj = DirectX::XMMatrixMultiply(mLightView, mLightProj);
					DirectX::XMStoreFloat4x4(&pPointLightData[i].m_mViewProj[face], DirectX::XMMatrixTranspose(mLightViewProj));
				}

				pPointLightData[i].m_FirstShadowMapTexId = firstTextureId;
				firstTextureId += 6;
			}
			m_AllPointLights->UnmapResource(0);

			pGfxContext->ResourceTransitionBarrier(m_VisiblePointLights.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			m_LightCullingPass->Apply(pGfxContext.Get(), &m_RenderContext, pScene);
			pGfxContext->SetComputeRootStructuredBuffer(1, m_AllPointLights.get());
			pGfxContext->SetComputeRootRWStructuredBuffer(2, m_VisiblePointLights.get());
			m_LightCullingPass->Exec(pGfxContext.Get());

			pGfxContext->ResourceTransitionBarrier(m_VisiblePointLights.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

			pGfxContext->PIXEndEvent();
		}

		{
			pGfxContext->PIXBeginEvent(L"TiledShading");

			pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer0.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
			pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer1.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
			pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer2.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
			pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer3.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
			pGfxContext->ResourceTransitionBarrier(m_SceneDepthSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
			pGfxContext->ResourceTransitionBarrier(m_LightingSurface.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			for (int i = 0; i < pScene->GetDirectionalLights().size(); ++i)
			{
				auto directionalLight = pScene->GetDirectionalLights()[i];

				DX12DepthSurface* pShadowMapForDirLight = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
				DX12ColorSurface* pRSMIntensitySurface = m_RenderContext.AcquireRSMRadiantIntensitySurfaceForDirectionalLight(directionalLight.get());
				DX12ColorSurface* pRSMNormalSurface = m_RenderContext.AcquireRSMNormalSurfaceForDirectionalLight(directionalLight.get());
				DX12ColorSurface* pEVSMSurface = m_RenderContext.AcquireEVSMSurfaceForDirectionalLight(directionalLight.get());
				pGfxContext->ResourceTransitionBarrier(pShadowMapForDirLight, D3D12_RESOURCE_STATE_GENERIC_READ);
				pGfxContext->ResourceTransitionBarrier(pRSMIntensitySurface, D3D12_RESOURCE_STATE_GENERIC_READ);
				pGfxContext->ResourceTransitionBarrier(pRSMNormalSurface, D3D12_RESOURCE_STATE_GENERIC_READ);
				pGfxContext->ResourceTransitionBarrier(pEVSMSurface, D3D12_RESOURCE_STATE_GENERIC_READ);
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

			m_TiledShadingPass->Apply(pGfxContext.Get(), &m_RenderContext, pScene);
			pGfxContext->SetComputeRootStructuredBuffer(1, m_AllDirectionalLights.get());
			pGfxContext->SetComputeRootStructuredBuffer(2, m_AllPointLights.get());
			pGfxContext->SetComputeRootStructuredBuffer(3, m_VisiblePointLights.get());
			pGfxContext->SetComputeDynamicCbvSrvUav(4, 0, m_SceneGBuffer0->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetComputeDynamicCbvSrvUav(4, 1, m_SceneGBuffer1->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetComputeDynamicCbvSrvUav(4, 2, m_SceneGBuffer2->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetComputeDynamicCbvSrvUav(4, 3, m_SceneGBuffer3->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetComputeDynamicCbvSrvUav(4, 4, m_SceneDepthSurface->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetComputeDynamicCbvSrvUav(4, 5, m_LightingSurface->GetStagingUAV().GetCpuHandle());
			int firstTextureId = 0;
			for (int i = 0; i < pScene->GetDirectionalLights().size(); ++i)
			{
				auto directionalLight = pScene->GetDirectionalLights()[i];

				DX12DepthSurface* pShadowMapForDirLight = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
				DX12ColorSurface* pRSMIntensitySurface = m_RenderContext.AcquireRSMRadiantIntensitySurfaceForDirectionalLight(directionalLight.get());
				DX12ColorSurface* pRSMNormalSurface = m_RenderContext.AcquireRSMNormalSurfaceForDirectionalLight(directionalLight.get());
				DX12ColorSurface* pEVSMSurface = m_RenderContext.AcquireEVSMSurfaceForDirectionalLight(directionalLight.get());
				pGfxContext->SetComputeDynamicCbvSrvUav(5, firstTextureId, pShadowMapForDirLight->GetStagingSRV().GetCpuHandle());
				pGfxContext->SetComputeDynamicCbvSrvUav(5, firstTextureId + 1, pRSMIntensitySurface->GetStagingSRV().GetCpuHandle());
				pGfxContext->SetComputeDynamicCbvSrvUav(5, firstTextureId + 2, pRSMNormalSurface->GetStagingSRV().GetCpuHandle());
				pGfxContext->SetComputeDynamicCbvSrvUav(5, firstTextureId + 3, pEVSMSurface->GetStagingSRV().GetCpuHandle());

				firstTextureId += 4;
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
			m_TiledShadingPass->Exec(pGfxContext.Get());

			pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer0.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
			pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer1.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
			pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer2.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
			pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer3.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
			pGfxContext->ResourceTransitionBarrier(m_SceneDepthSurface.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
			pGfxContext->ResourceTransitionBarrier(m_LightingSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

			for (int i = 0; i < pScene->GetDirectionalLights().size(); ++i)
			{
				auto directionalLight = pScene->GetDirectionalLights()[i];

				DX12DepthSurface* pShadowMapForDirLight = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
				DX12ColorSurface* pRSMIntensitySurface = m_RenderContext.AcquireRSMRadiantIntensitySurfaceForDirectionalLight(directionalLight.get());
				DX12ColorSurface* pRSMNormalSurface = m_RenderContext.AcquireRSMNormalSurfaceForDirectionalLight(directionalLight.get());
				DX12ColorSurface* pEVSMSurface = m_RenderContext.AcquireEVSMSurfaceForDirectionalLight(directionalLight.get());
				pGfxContext->ResourceTransitionBarrier(pShadowMapForDirLight, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				pGfxContext->ResourceTransitionBarrier(pRSMIntensitySurface, D3D12_RESOURCE_STATE_RENDER_TARGET);
				pGfxContext->ResourceTransitionBarrier(pRSMNormalSurface, D3D12_RESOURCE_STATE_RENDER_TARGET);
				pGfxContext->ResourceTransitionBarrier(pEVSMSurface, D3D12_RESOURCE_STATE_RENDER_TARGET);
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

		pGfxContext->ClearRenderTarget(m_LightingSurface.Get(), 0, 0, 0, 0);

		// setup color and depth buffers
		DX12ColorSurface* pColorSurfaces[] = { m_LightingSurface.Get() };
		pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, nullptr);

		pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		pGfxContext->SetViewport(0, 0, m_Width, m_Height);

		m_RenderContext.SetModelMatrix(DirectX::XMMatrixIdentity());
		m_RenderContext.SetViewMatrix(pCamera->GetViewMatrix());
		m_RenderContext.SetProjMatrix(pCamera->GetProjectionMatrix());

		pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer0.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
		pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer1.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
		pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer2.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
		pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer3.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
		pGfxContext->ResourceTransitionBarrier(m_SceneDepthSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);

		for (auto directionalLight : pScene->GetDirectionalLights())
		{
			DX12DepthSurface* pShadowMapForDirLight = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
			DX12ColorSurface* pRSMIntensitySurface = m_RenderContext.AcquireRSMRadiantIntensitySurfaceForDirectionalLight(directionalLight.get());
			DX12ColorSurface* pRSMNormalSurface = m_RenderContext.AcquireRSMNormalSurfaceForDirectionalLight(directionalLight.get());
			DX12ColorSurface* pEVSMSurface = m_RenderContext.AcquireEVSMSurfaceForDirectionalLight(directionalLight.get());
			pGfxContext->ResourceTransitionBarrier(pShadowMapForDirLight, D3D12_RESOURCE_STATE_GENERIC_READ);
			pGfxContext->ResourceTransitionBarrier(pRSMIntensitySurface, D3D12_RESOURCE_STATE_GENERIC_READ);
			pGfxContext->ResourceTransitionBarrier(pRSMNormalSurface, D3D12_RESOURCE_STATE_GENERIC_READ);
			pGfxContext->ResourceTransitionBarrier(pEVSMSurface, D3D12_RESOURCE_STATE_GENERIC_READ);

			m_DirLightFilter2D->Apply(pGfxContext.Get(), &m_RenderContext, directionalLight.get());

			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 0, m_SceneGBuffer0->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 1, m_SceneGBuffer1->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 2, m_SceneGBuffer2->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 3, m_SceneGBuffer3->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 4, m_SceneDepthSurface->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 5, pShadowMapForDirLight->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 6, pRSMIntensitySurface->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 7, pRSMNormalSurface->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 8, pEVSMSurface->GetStagingSRV().GetCpuHandle());

			m_DirLightFilter2D->Draw(pGfxContext.Get());

			pGfxContext->ResourceTransitionBarrier(pShadowMapForDirLight, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			pGfxContext->ResourceTransitionBarrier(pRSMIntensitySurface, D3D12_RESOURCE_STATE_RENDER_TARGET);
			pGfxContext->ResourceTransitionBarrier(pRSMNormalSurface, D3D12_RESOURCE_STATE_RENDER_TARGET);
			pGfxContext->ResourceTransitionBarrier(pEVSMSurface, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		for (auto pointLight : pScene->GetPointLights())
		{
			auto pShadowMapsForPointLight = m_RenderContext.AcquireDepthSurfaceForPointLight(pointLight.get());

			for (int i = 0; i < pShadowMapsForPointLight.size(); ++i)
			{
				pGfxContext->ResourceTransitionBarrier(pShadowMapsForPointLight[i], D3D12_RESOURCE_STATE_GENERIC_READ);
			}

			m_PointLightFilter2D->Apply(pGfxContext.Get(), &m_RenderContext, pointLight.get());

			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 0, m_SceneGBuffer0->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 1, m_SceneGBuffer1->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 2, m_SceneGBuffer2->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 3, m_SceneGBuffer3->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 4, m_SceneDepthSurface->GetStagingSRV().GetCpuHandle());
			for (int i = 0; i < pShadowMapsForPointLight.size(); ++i)
			{
				pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 5 + i, pShadowMapsForPointLight[i]->GetStagingSRV().GetCpuHandle());
			}

			m_DirLightFilter2D->Draw(pGfxContext.Get());

			for (int i = 0; i < pShadowMapsForPointLight.size(); ++i)
			{
				pGfxContext->ResourceTransitionBarrier(pShadowMapsForPointLight[i], D3D12_RESOURCE_STATE_DEPTH_WRITE);
			}
		}

		pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer0.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer1.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer2.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer3.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		pGfxContext->ResourceTransitionBarrier(m_SceneDepthSurface.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

		pGfxContext->PIXEndEvent();
	}
}

void Renderer::RenderGBuffer(const Camera* pCamera, Scene* pScene)
{
	m_RenderContext.SetShadingCfg(ShadingConfiguration_GBuffer);

	DX12ScopedGraphicsContext pGfxContext;

	pGfxContext->PIXBeginEvent(L"G-Buffer");

    // Clear the views.
    pGfxContext->ClearRenderTarget(m_SceneGBuffer0.Get(), 0, 0, 0, 0);
    pGfxContext->ClearRenderTarget(m_SceneGBuffer1.Get(), 0, 0, 0, 0);
    pGfxContext->ClearRenderTarget(m_SceneGBuffer2.Get(), 0, 0, 0, 0);
    pGfxContext->ClearRenderTarget(m_SceneGBuffer3.Get(), 0, 0, 0, 0);
	pGfxContext->ClearDepthTarget(m_SceneDepthSurface.Get(), 1.0f);

	// setup color and depth buffers
	DX12ColorSurface* pColorSurfaces[] = {
		m_SceneGBuffer0.Get(),
		m_SceneGBuffer1.Get(),
		m_SceneGBuffer2.Get(),
		m_SceneGBuffer3.Get()
	};
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, m_SceneDepthSurface.Get());

	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

	m_RenderContext.SetModelMatrix(DirectX::XMMatrixIdentity());
	m_RenderContext.SetViewMatrix(pCamera->GetViewMatrix());
	m_RenderContext.SetProjMatrix(pCamera->GetProjectionMatrix());

	for (auto model : pScene->GetModels())
	{
		model->DrawPrimitives(&m_RenderContext, pGfxContext.Get());
	}

	pGfxContext->PIXEndEvent();

    m_RenderContext.SaveInfoForNextFrame();
}

void Renderer::PostProcess(const Camera* pCamera, Scene* pScene)
{
    AAFilter();
    CalcAvgLuminance(m_LightingSurface.Get());
    ToneMap();
}

void Renderer::ResolveToSwapChain()
{
	m_SwapChain->Begin();

	DX12ScopedSwapChainContext pGfxContext;

    pGfxContext->PIXBeginEvent(L"ResolveToSwapChain");

    // Clear the views.
	DX12ColorSurface* pColorSurface = m_SwapChain->GetBackBuffer();
	DX12ColorSurface* pColorSurfaces[] = { pColorSurface };
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, nullptr);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

    pGfxContext->ResourceTransitionBarrier(m_LDRSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
    m_ResolveToSwapChainFilter2D->Apply(pGfxContext.Get());
    pGfxContext->SetGraphicsRootDescriptorTable(0, m_LDRSurface->GetSRV());
    m_ResolveToSwapChainFilter2D->Draw(pGfxContext.Get());
    pGfxContext->ResourceTransitionBarrier(m_LDRSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    pGfxContext->PIXEndEvent();
}

void Renderer::RenderDebugMenu()
{
	DX12ScopedSwapChainContext pGfxContext;

	pGfxContext->PIXBeginEvent(L"DebugMenu");

    // Clear the views.
	DX12ColorSurface* pColorSurface = m_SwapChain->GetBackBuffer();
	DX12ColorSurface* pColorSurfaces[] = { pColorSurface };
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, nullptr);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

	EngineTuning::Display(*pGfxContext, 10.0f, 40.0f, 1900.0f, 1040.0f);

	pGfxContext->PIXEndEvent();
}

void Renderer::AAFilter()
{
    DX12ScopedGraphicsContext pGfxContext;

    pGfxContext->PIXBeginEvent(L"AAFilter");

    pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pGfxContext->SetViewport(0, 0, m_Width, m_Height);

    {
        pGfxContext->ResourceTransitionBarrier(m_LightingSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
        pGfxContext->ResourceTransitionBarrier(m_HistoryLightingSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
        pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer3.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
        m_AntiAliasingFilter2D->Apply(pGfxContext.Get());
        pGfxContext->SetGraphicsDynamicCbvSrvUav(0, 0, m_LightingSurface->GetStagingSRV().GetCpuHandle());
        pGfxContext->SetGraphicsDynamicCbvSrvUav(0, 1, m_HistoryLightingSurface->GetStagingSRV().GetCpuHandle());
        pGfxContext->SetGraphicsDynamicCbvSrvUav(0, 2, m_SceneGBuffer3->GetStagingSRV().GetCpuHandle());
        DX12ColorSurface* pSurfaces[] = {m_PostProcessSurface.Get()};
        pGfxContext->SetRenderTargets(_countof(pSurfaces), pSurfaces, nullptr);
        m_AntiAliasingFilter2D->Draw(pGfxContext.Get());
        pGfxContext->ResourceTransitionBarrier(m_LightingSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
        pGfxContext->ResourceTransitionBarrier(m_HistoryLightingSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
        pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer3.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    {
        pGfxContext->ResourceTransitionBarrier(m_PostProcessSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
        m_IdentityFilter2D->Apply(pGfxContext.Get());
        pGfxContext->SetGraphicsDynamicCbvSrvUav(0, 0, m_PostProcessSurface->GetStagingSRV().GetCpuHandle());
        DX12ColorSurface* pSurfaces[] = {m_HistoryLightingSurface.Get()};
        pGfxContext->SetRenderTargets(_countof(pSurfaces), pSurfaces, nullptr);
        m_IdentityFilter2D->Draw(pGfxContext.Get());
        pGfxContext->ResourceTransitionBarrier(m_PostProcessSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    }

    pGfxContext->PIXEndEvent();
}

void Renderer::CalcAvgLuminance(DX12ColorSurface* surf)
{
    DX12ScopedGraphicsContext pGfxContext;

    pGfxContext->PIXBeginEvent(L"Average Luminance");

    pGfxContext->ResourceTransitionBarrier(surf, D3D12_RESOURCE_STATE_GENERIC_READ);
    pGfxContext->ResourceTransitionBarrier((*m_LuminanceSurfaces.begin()).Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    m_ReduceLuminanceInitial->Apply(pGfxContext.Get());
    pGfxContext->SetComputeDynamicCbvSrvUav(0, 0, surf->GetStagingSRV().GetCpuHandle());
    pGfxContext->SetComputeDynamicCbvSrvUav(0, 1, (*m_LuminanceSurfaces.begin())->GetStagingUAV().GetCpuHandle());
    m_ReduceLuminanceInitial->Dispatch(pGfxContext.Get(), (*m_LuminanceSurfaces.begin())->GetWidth(), (*m_LuminanceSurfaces.begin())->GetHeight());
    pGfxContext->ResourceTransitionBarrier(surf, D3D12_RESOURCE_STATE_RENDER_TARGET);

    for (int32_t i = 1; i < m_LuminanceSurfaces.size(); ++i)
    {
        DX12ColorSurface* preSurf = m_LuminanceSurfaces[i - 1].Get();
        DX12ColorSurface* curSurf = m_LuminanceSurfaces[i].Get();

        std::shared_ptr<ComputeProcessing> curProcessing;
        if (i == (m_LuminanceSurfaces.size() - 1))
        {
            curProcessing = m_ReduceLuminanceFinal;
        }
        else
        {
           curProcessing = m_ReduceLuminance;
        }

        pGfxContext->ResourceTransitionBarrier(preSurf, D3D12_RESOURCE_STATE_GENERIC_READ);
        pGfxContext->ResourceTransitionBarrier(curSurf, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        curProcessing->Apply(pGfxContext.Get());
        pGfxContext->SetComputeDynamicCbvSrvUav(0, 0, preSurf->GetStagingSRV().GetCpuHandle());
        pGfxContext->SetComputeDynamicCbvSrvUav(0, 1, curSurf->GetStagingUAV().GetCpuHandle());
        curProcessing->Dispatch(pGfxContext.Get(), curSurf->GetWidth(), curSurf->GetHeight());        
    }

    pGfxContext->PIXEndEvent();
}

void Renderer::ToneMap()
{
	DX12ScopedGraphicsContext pGfxContext;

    pGfxContext->PIXBeginEvent(L"ToneMap");

    // Clear the views.
	DX12ColorSurface* pColorSurfaces[] = { m_LDRSurface.Get() };
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, nullptr);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

    pGfxContext->ResourceTransitionBarrier(m_PostProcessSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
    pGfxContext->ResourceTransitionBarrier(m_LuminanceSurfaces.back().Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
    m_ToneMapFilter2D->Apply(pGfxContext.Get());

    HLSL::CameraSettings constants;
    constants.m_ToneMapEnabled = g_ToneMapping;
    constants.m_ExposureMode = ExposureModes_Automatic;
    constants.m_KeyValue = g_ToneMapTargetLuminance;
    pGfxContext->SetGraphicsRootDynamicConstantBufferView(0, &constants, sizeof(constants));

    pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 0, m_PostProcessSurface->GetStagingSRV().GetCpuHandle());
    pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 1, m_LuminanceSurfaces.back()->GetStagingSRV().GetCpuHandle());

    m_ToneMapFilter2D->Draw(pGfxContext.Get());

	pGfxContext->ResourceTransitionBarrier(m_PostProcessSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    pGfxContext->PIXEndEvent();
}
