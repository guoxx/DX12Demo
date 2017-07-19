#include "pch.h"
#include "Renderer.h"

#include "Camera.h"
#include "Scene.h"
#include "Model.h"

#include "Lights/PointLight.h"
#include "Lights/DirectionalLight.h"

#include "Filters/ComputeProcessing.h"
#include "Filters/ImageProcessing.h"

#include "Pass/LightCullingPass.h"
#include "Pass/TiledShadingPass.h"

#include "Shaders/CompiledShaders/Passthrough.h"
#include "Shaders/CompiledShaders/LuminanceReductionInitial.h"
#include "Shaders/CompiledShaders/LuminanceReduction.h"
#include "Shaders/CompiledShaders/LuminanceReductionFinal.h"
#include <Shaders/CompiledShaders/EVSM.h>


Renderer::Renderer(GFX_HWND hwnd, int32_t width, int32_t height)
	: m_Width{ width}
	, m_Height{ height }
    , m_FrameIdx{ 0 }
{
	DX12Device* pDevice = DX12GraphicsManager::GetInstance()->GetDevice();

	m_SwapChain = std::make_shared<DX12SwapChain>(pDevice, hwnd, width, height, GFX_FORMAT_SWAPCHAIN);

	m_GBuffer.m_GBuffer[0] = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R8G8B8A8_UNORM, width, height));
	m_GBuffer.m_GBuffer[1] = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R8G8B8A8_UNORM, width, height));
	m_GBuffer.m_GBuffer[2] = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R8G8B8A8_UNORM, width, height));
	m_GBuffer.m_GBuffer[3] = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R16G16B16A16_FLOAT, width, height));
	m_GBuffer.m_DepthSurface = RenderableSurfaceManager::GetInstance()->AcquireDepthSurface(RenderableSurfaceDesc(GFX_FORMAT_D32_FLOAT, width, height));

    m_PostProcessSurfaces.m_ScreenSpaceShadowsSurface = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R32_FLOAT, width, height));

	m_PostProcessSurfaces.m_HDRSurface = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_HDR, width, height));
	m_PostProcessSurfaces.m_HistoryHDRSurface = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_HDR, width, height));
	m_PostProcessSurfaces.m_AASurface = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_HDR, width, height));

	m_PostProcessSurfaces.m_LDRSurface = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_LDR, width, height));

    {
        auto psoSetup = [](DX12GraphicsPsoDesc& desc)
        {
            desc.SetRenderTargetFormat(GFX_FORMAT_R32G32B32A32_FLOAT.RTVFormat);
        };
        m_EVSM = std::make_shared<ImageProcessing>(pDevice, g_EVSM_VS_bytecode, g_EVSM_PS_bytecode, psoSetup);
    }

    uint32_t lumReductionWidth = m_Width;
    uint32_t lumReductionHeight = m_Height;
    while (true)
    {
        lumReductionWidth = DX::DivideByMultiple(lumReductionWidth, LUMINANCE_REDUCTION_THREAD_GROUP_SIZE);
        lumReductionHeight = DX::DivideByMultiple(lumReductionHeight, LUMINANCE_REDUCTION_THREAD_GROUP_SIZE);

	    auto surf = RenderableSurfaceManager::GetInstance()->AcquireColorSurface(RenderableSurfaceDesc(GFX_FORMAT_R32_FLOAT, lumReductionWidth, lumReductionHeight));
        m_PostProcessSurfaces.m_LuminanceSurfaces.push_back(surf);

        if (lumReductionWidth == 1 && lumReductionHeight == 1)
        {
            break;
        }
    }
    assert(m_PostProcessSurfaces.m_LuminanceSurfaces.size() >= 2);

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

    m_ScreenSpaceShadowPass = std::make_shared<ScreenSpaceShadowsPass>(pDevice);
    m_PointLightShadingPass = std::make_shared<PointLightShadingPass>(pDevice);
    m_DirectionalLightShadingPass = std::make_shared<DirectionalLightShadingPass>(pDevice);

    m_TemporalAA = std::make_shared<TemporalAAPass>(pDevice);
    m_ToneMap = std::make_shared<ToneMappingPass>(pDevice);

    {
        auto psoSetup = [](DX12GraphicsPsoDesc& desc)
        {
            desc.SetRenderTargetFormat(GFX_FORMAT_SWAPCHAIN.RTVFormat);
        };
        m_ResolveToSwapChain = std::make_shared<ImageProcessing>(pDevice, g_Passthrough_VS_bytecode, g_Passthrough_PS_bytecode, psoSetup);
    }

    m_ReduceLuminanceInitial = std::make_shared<ComputeProcessing>(pDevice, g_LuminanceReductionInitial_CS_bytecode);
    m_ReduceLuminance = std::make_shared<ComputeProcessing>(pDevice, g_LuminanceReduction_CS_bytecode);
    m_ReduceLuminanceFinal = std::make_shared<ComputeProcessing>(pDevice, g_LuminanceReductionFinal_CS_bytecode);
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
    ScreenSpaceShadows(pCamera, pScene);
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
	DX12ScopedGraphicsContext pGfxContext{L"RenderShadowMaps"};

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
		GPU_MARKER(pGfxContext, ShadowMap_DirectionalLight);

        directionalLight->PrepareForShadowPass(pCamera, DX12DirectionalLightShadowMapSize);

		if (g_RSMEnabled)
		{
			m_RenderContext.SetCurrentLightForRSM(directionalLight.get());

			auto pIntensitySurface = m_RenderContext.AcquireRSMRadiantIntensitySurfaceForDirectionalLight(directionalLight.get());
			auto pNormalSurface = m_RenderContext.AcquireRSMNormalSurfaceForDirectionalLight(directionalLight.get());
			auto pDepthSurface = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());

			pGfxContext->ResourceTransitionBarrier(pDepthSurface.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

			pGfxContext->ClearRenderTarget(pIntensitySurface.Get(), 0, 0, 0, 0);
			pGfxContext->ClearRenderTarget(pNormalSurface.Get(), 1, 0, 0, 0);
			pGfxContext->ClearDepthTarget(pDepthSurface.Get(), 1.0f);

			DX12ColorSurface* pSurfaces[] = { pIntensitySurface.Get(), pNormalSurface.Get() };
			pGfxContext->SetRenderTargets(_countof(pSurfaces), pSurfaces, pDepthSurface.Get());
		}
		else
		{
			auto pDepthSurface = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
			pGfxContext->ResourceTransitionBarrier(pDepthSurface.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

			pGfxContext->ClearDepthTarget(pDepthSurface.Get(), 1.0f);
			pGfxContext->SetRenderTargets(0, nullptr, pDepthSurface.Get());
		}

        for (int32_t cascadeIdx = 0; cascadeIdx < DirectionalLight::NUM_CASCADED_SHADOW_MAP; ++cascadeIdx)
        {
            static_assert(DirectionalLight::NUM_CASCADED_SHADOW_MAP == 4, "");
            static const wchar_t* markers[4] = {L"Cascade - 0", L"Cascade - 1", L"Cascade - 2", L"Cascade - 3"};
            GPU_MARKER_NAMED(pGfxContext, markers[cascadeIdx]);

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
        }

		if (g_EVSMEnabled)
		{
            GPU_MARKER(pGfxContext, EVSM);

			auto pDepthSurface = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
			auto pEVSMSurface = m_RenderContext.AcquireEVSMSurfaceForDirectionalLight(directionalLight.get());

			pGfxContext->SetRenderTarget(pEVSMSurface.Get());

		    pGfxContext->SetViewport(0, 0, DX12DirectionalLightShadowMapSize, DX12DirectionalLightShadowMapSize);

			pGfxContext->ResourceTransitionBarrier(pDepthSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
			pGfxContext->ResourceTransitionBarrier(pEVSMSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

			m_EVSM->Apply(pGfxContext.Get());
		    HLSL::EVSMConstants constants;
		    constants.m_EVSM.m_Enabled = g_EVSMEnabled ? 1 : 0;
		    constants.m_EVSM.m_PositiveExponent = g_EVSMPositiveExponent;
		    constants.m_EVSM.m_NegativeExponent = g_EVSMNegativeExponent;
		    constants.m_EVSM.m_LightBleedingReduction = g_LightBleedingReduction;
		    constants.m_EVSM.m_VSMBias = g_VSMBias;
		    pGfxContext->SetGraphicsRootDynamicConstantBufferView(0, &constants, sizeof(constants));
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 0, pDepthSurface->GetStagingSRV().GetCpuHandle());
			m_EVSM->Draw(pGfxContext.Get());
		}
	}

	for (auto pointLight : pScene->GetPointLights())
	{
		GPU_MARKER(pGfxContext, ShadowMap_PointLight);

        pointLight->PrepareForShadowPass(pCamera, DX12PointLightShadowMapSize);

		for (int i = 0; i < 6; ++i)
		{
			wchar_t* axisNames[] = { L"POSITIVE_X", L"NEGATIVE_X", L"POSITIVE_Y", L"NEGATIVE_Y", L"POSITIVE_Z", L"NEGATIVE_Z" };
			GPU_MARKER_NAMED(pGfxContext, axisNames[i]);

			if (g_RSMEnabled)
			{
				m_RenderContext.SetCurrentLightForRSM(pointLight.get());

				auto pIntensitySurfaces = m_RenderContext.AcquireRSMRadiantIntensitySurfaceForPointLight(pointLight.get());
				auto pNormalSurfaces = m_RenderContext.AcquireRSMNormalSurfaceForPointLight(pointLight.get());
				auto pDepthSurfaces = m_RenderContext.AcquireDepthSurfaceForPointLight(pointLight.get());

				pGfxContext->ClearRenderTarget(pIntensitySurfaces[i].Get(), 0, 0, 0, 0);
				pGfxContext->ClearRenderTarget(pNormalSurfaces[i].Get(), 1, 0, 0, 0);
				pGfxContext->ClearDepthTarget(pDepthSurfaces[i].Get(), 1.0f);

				DX12ColorSurface* pSurfaces[] = { pIntensitySurfaces[i].Get(), pNormalSurfaces[i].Get() };
				pGfxContext->SetRenderTargets(_countof(pSurfaces), pSurfaces, pDepthSurfaces[i].Get());
			}
			else
			{
				auto pDepthSurfaces = m_RenderContext.AcquireDepthSurfaceForPointLight(pointLight.get());

                PointLightShadowMapSet shadowmapSet{pDepthSurfaces};
                shadowmapSet.TransmitToWrite(pGfxContext.Get());

				pGfxContext->ClearDepthTarget(pDepthSurfaces[i].Get(), 1.0f);
				pGfxContext->SetRenderTargets(0, nullptr, pDepthSurfaces[i].Get());
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
		}
	}
}

void Renderer::DeferredLighting(const Camera* pCamera, Scene* pScene)
{
	DX12ScopedGraphicsContext pGfxContext{L"DeferredLighting"};

    pGfxContext->ResourceTransitionBarrier(m_PostProcessSurfaces.m_ScreenSpaceShadowsSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);

	if (g_TiledShading)
	{
		{
			GPU_MARKER(pGfxContext, LightCulling);

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
		}

		{
			GPU_MARKER(pGfxContext, TiledShading);

            m_GBuffer.TransmitToRead(pGfxContext.Get());
			pGfxContext->ResourceTransitionBarrier(m_PostProcessSurfaces.m_HDRSurface.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			for (int i = 0; i < pScene->GetDirectionalLights().size(); ++i)
			{
				auto directionalLight = pScene->GetDirectionalLights()[i];

				auto pShadowMapForDirLight = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
				auto pRSMIntensitySurface = m_RenderContext.AcquireRSMRadiantIntensitySurfaceForDirectionalLight(directionalLight.get());
				auto pRSMNormalSurface = m_RenderContext.AcquireRSMNormalSurfaceForDirectionalLight(directionalLight.get());
				auto pEVSMSurface = m_RenderContext.AcquireEVSMSurfaceForDirectionalLight(directionalLight.get());
				pGfxContext->ResourceTransitionBarrier(pShadowMapForDirLight.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
				pGfxContext->ResourceTransitionBarrier(pRSMIntensitySurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
				pGfxContext->ResourceTransitionBarrier(pRSMNormalSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
				pGfxContext->ResourceTransitionBarrier(pEVSMSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
			}
			for (int i = 0; i < pScene->GetPointLights().size(); ++i)
			{
				auto pointLight = pScene->GetPointLights()[i];
				auto pShadowMapsForPointLight = m_RenderContext.AcquireDepthSurfaceForPointLight(pointLight.get());
				for (int j = 0; j < pShadowMapsForPointLight.size(); ++j)
				{
					pGfxContext->ResourceTransitionBarrier(pShadowMapsForPointLight[j].Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
				}
			}

			m_TiledShadingPass->Apply(pGfxContext.Get(), &m_RenderContext, pScene);
			pGfxContext->SetComputeRootStructuredBuffer(1, m_AllDirectionalLights.get());
			pGfxContext->SetComputeRootStructuredBuffer(2, m_AllPointLights.get());
			pGfxContext->SetComputeRootStructuredBuffer(3, m_VisiblePointLights.get());
            m_GBuffer.SetAsSRV(pGfxContext.Get(), 4, 0);
            pGfxContext->SetComputeDynamicCbvSrvUav(4, 5, m_PostProcessSurfaces.m_ScreenSpaceShadowsSurface->GetStagingSRV().GetCpuHandle());
			pGfxContext->SetComputeDynamicCbvSrvUav(4, 6, m_PostProcessSurfaces.m_HDRSurface->GetStagingUAV().GetCpuHandle());
			int firstTextureId = 0;
			for (int i = 0; i < pScene->GetDirectionalLights().size(); ++i)
			{
				auto directionalLight = pScene->GetDirectionalLights()[i];

				auto pShadowMapForDirLight = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
				auto pRSMIntensitySurface = m_RenderContext.AcquireRSMRadiantIntensitySurfaceForDirectionalLight(directionalLight.get());
				auto pRSMNormalSurface = m_RenderContext.AcquireRSMNormalSurfaceForDirectionalLight(directionalLight.get());
				auto pEVSMSurface = m_RenderContext.AcquireEVSMSurfaceForDirectionalLight(directionalLight.get());
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

			pGfxContext->ResourceTransitionBarrier(m_PostProcessSurfaces.m_HDRSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

			for (int i = 0; i < pScene->GetDirectionalLights().size(); ++i)
			{
				auto directionalLight = pScene->GetDirectionalLights()[i];

				auto pShadowMapForDirLight = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
				auto pRSMIntensitySurface = m_RenderContext.AcquireRSMRadiantIntensitySurfaceForDirectionalLight(directionalLight.get());
				auto pRSMNormalSurface = m_RenderContext.AcquireRSMNormalSurfaceForDirectionalLight(directionalLight.get());
				auto pEVSMSurface = m_RenderContext.AcquireEVSMSurfaceForDirectionalLight(directionalLight.get());
				pGfxContext->ResourceTransitionBarrier(pShadowMapForDirLight.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
				pGfxContext->ResourceTransitionBarrier(pRSMIntensitySurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
				pGfxContext->ResourceTransitionBarrier(pRSMNormalSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
				pGfxContext->ResourceTransitionBarrier(pEVSMSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
			}
			for (int i = 0; i < pScene->GetPointLights().size(); ++i)
			{
				auto pointLight = pScene->GetPointLights()[i];
				auto pShadowMapsForPointLight = m_RenderContext.AcquireDepthSurfaceForPointLight(pointLight.get());
				for (int j = 0; j < pShadowMapsForPointLight.size(); ++j)
				{
					pGfxContext->ResourceTransitionBarrier(pShadowMapsForPointLight[j].Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
				}
			}
		}
	}
	else
	{
        GPU_MARKER(pGfxContext, DeferredLighting);

        pGfxContext->ResourceTransitionBarrier(m_PostProcessSurfaces.m_HDRSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		pGfxContext->ClearRenderTarget(m_PostProcessSurfaces.m_HDRSurface.Get(), 0, 0, 0, 0);
		pGfxContext->SetRenderTarget(m_PostProcessSurfaces.m_HDRSurface.Get());

		pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pGfxContext->SetViewport(0, 0, m_Width, m_Height);

		m_RenderContext.SetModelMatrix(DirectX::XMMatrixIdentity());
		m_RenderContext.SetViewMatrix(pCamera->GetViewMatrix());
		m_RenderContext.SetProjMatrix(pCamera->GetProjectionMatrix());

		for (auto directionalLight : pScene->GetDirectionalLights())
		{
			auto pShadowMapForDirLight = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(directionalLight.get());
			auto pRSMIntensitySurface = m_RenderContext.AcquireRSMRadiantIntensitySurfaceForDirectionalLight(directionalLight.get());
			auto pRSMNormalSurface = m_RenderContext.AcquireRSMNormalSurfaceForDirectionalLight(directionalLight.get());
			auto pEVSMSurface = m_RenderContext.AcquireEVSMSurfaceForDirectionalLight(directionalLight.get());

            DirectionalLightShadowMapSet shadowmapSet{pShadowMapForDirLight, pRSMIntensitySurface, pRSMNormalSurface, pEVSMSurface};
            m_DirectionalLightShadingPass->Apply(pGfxContext.Get(), &m_RenderContext, directionalLight.get(), m_GBuffer, shadowmapSet, m_PostProcessSurfaces);
		}

		for (auto pointLight : pScene->GetPointLights())
		{
			auto pShadowMapsForPointLight = m_RenderContext.AcquireDepthSurfaceForPointLight(pointLight.get());
            PointLightShadowMapSet shadowmapSet{pShadowMapsForPointLight};
            m_PointLightShadingPass->Apply(pGfxContext.Get(), &m_RenderContext, pointLight.get(), m_GBuffer, shadowmapSet, m_PostProcessSurfaces);
		}
	}
}

void Renderer::RenderGBuffer(const Camera* pCamera, Scene* pScene)
{
	m_RenderContext.SetShadingCfg(ShadingConfiguration_GBuffer);

	DX12ScopedGraphicsContext pGfxContext{L"RenderGBuffer"};

	GPU_MARKER(pGfxContext, G_Buffer);

    m_GBuffer.TransmitToWrite(pGfxContext.Get());

    // Clear the views.
    for (auto surf : m_GBuffer.m_GBuffer)
    {
        pGfxContext->ClearRenderTarget(surf.Get(), 0, 0, 0, 0);
    }
	pGfxContext->ClearDepthTarget(m_GBuffer.m_DepthSurface.Get(), 1.0f);

    m_GBuffer.SetAsRTV(pGfxContext.Get());

	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

	m_RenderContext.SetModelMatrix(DirectX::XMMatrixIdentity());
	m_RenderContext.SetViewMatrix(pCamera->GetViewMatrix());
	m_RenderContext.SetProjMatrix(pCamera->GetProjectionMatrix());

	for (auto model : pScene->GetModels())
	{
		model->DrawPrimitives(&m_RenderContext, pGfxContext.Get());
	}

    m_RenderContext.SaveInfoForNextFrame();
}

void Renderer::PostProcess(const Camera* pCamera, Scene* pScene)
{
    AAFilter();
    CalcAvgLuminance(m_PostProcessSurfaces.m_HDRSurface.Get());
    ToneMap();
}

void Renderer::ResolveToSwapChain()
{
	m_SwapChain->Begin();

	DX12ScopedGraphicsContext pGfxContext{L"ResolveToSwapChain"};

    GPU_MARKER(pGfxContext, ResolveToSwapChain);

    // Clear the views.
	DX12ColorSurface* pColorSurface = m_SwapChain->GetBackBuffer();
	DX12ColorSurface* pColorSurfaces[] = { pColorSurface };
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, nullptr);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

    pGfxContext->ResourceTransitionBarrier(m_PostProcessSurfaces.m_LDRSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
    m_ResolveToSwapChain->Apply(pGfxContext.Get());
    pGfxContext->SetGraphicsRootDescriptorTable(0, m_PostProcessSurfaces.m_LDRSurface->GetSRV());
    m_ResolveToSwapChain->Draw(pGfxContext.Get());
}

void Renderer::RenderDebugMenu()
{
	DX12ScopedGraphicsContext pGfxContext{L"RenderDebugMenu"};

	GPU_MARKER(pGfxContext, DebugMenu);

    // Clear the views.
	DX12ColorSurface* pColorSurface = m_SwapChain->GetBackBuffer();
	DX12ColorSurface* pColorSurfaces[] = { pColorSurface };
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, nullptr);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

	EngineTuning::Display(*pGfxContext, 10.0f, 40.0f, 1900.0f, 1040.0f);
}

void Renderer::AAFilter()
{
    DX12ScopedGraphicsContext pGfxContext{L"AAFilter"};
    m_TemporalAA->Apply(pGfxContext.Get(), &m_RenderContext, m_GBuffer, m_PostProcessSurfaces);
}

void Renderer::CalcAvgLuminance(DX12ColorSurface* surf)
{
    DX12ScopedGraphicsContext pGfxContext{L"CalcAvgLuminance"};

    GPU_MARKER(pGfxContext, Average_Luminance);

    pGfxContext->ResourceTransitionBarrier(surf, D3D12_RESOURCE_STATE_GENERIC_READ);
    pGfxContext->ResourceTransitionBarrier((*m_PostProcessSurfaces.m_LuminanceSurfaces.begin()).Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    m_ReduceLuminanceInitial->Apply(pGfxContext.Get());
    pGfxContext->SetComputeDynamicCbvSrvUav(0, 0, surf->GetStagingSRV().GetCpuHandle());
    pGfxContext->SetComputeDynamicCbvSrvUav(0, 1, (*m_PostProcessSurfaces.m_LuminanceSurfaces.begin())->GetStagingUAV().GetCpuHandle());
    m_ReduceLuminanceInitial->Dispatch(pGfxContext.Get(), (*m_PostProcessSurfaces.m_LuminanceSurfaces.begin())->GetWidth(), (*m_PostProcessSurfaces.m_LuminanceSurfaces.begin())->GetHeight());
    pGfxContext->ResourceTransitionBarrier(surf, D3D12_RESOURCE_STATE_RENDER_TARGET);

    for (int32_t i = 1; i < m_PostProcessSurfaces.m_LuminanceSurfaces.size(); ++i)
    {
        DX12ColorSurface* preSurf = m_PostProcessSurfaces.m_LuminanceSurfaces[i - 1].Get();
        DX12ColorSurface* curSurf = m_PostProcessSurfaces.m_LuminanceSurfaces[i].Get();

        std::shared_ptr<ComputeProcessing> curProcessing;
        if (i == (m_PostProcessSurfaces.m_LuminanceSurfaces.size() - 1))
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
}

void Renderer::ToneMap()
{
	DX12ScopedGraphicsContext pGfxContext{L"ToneMap"};
    m_ToneMap->Apply(pGfxContext.Get(), &m_RenderContext, m_PostProcessSurfaces);
}

void Renderer::ScreenSpaceShadows(const Camera* pCamera, Scene* pScene)
{
    DX12ScopedGraphicsContext pGfxContext{ L"ScreenSpaceShadows" };

    pGfxContext->PIXBeginEvent(L"ScreenSpaceShadows");

    pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pGfxContext->SetViewport(0, 0, m_Width, m_Height);

    std::shared_ptr<DirectionalLight> sunLight = pScene->GetDirectionalLights()[0];

    auto pShadowMapForDirLight = m_RenderContext.AcquireDepthSurfaceForDirectionalLight(sunLight.get());
    auto pRSMIntensitySurface = m_RenderContext.AcquireRSMRadiantIntensitySurfaceForDirectionalLight(sunLight.get());
    auto pRSMNormalSurface = m_RenderContext.AcquireRSMNormalSurfaceForDirectionalLight(sunLight.get());
    auto pEVSMSurface = m_RenderContext.AcquireEVSMSurfaceForDirectionalLight(sunLight.get());
    DirectionalLightShadowMapSet shadowmapSet{pShadowMapForDirLight, pRSMIntensitySurface, pRSMNormalSurface, pEVSMSurface};

    m_ScreenSpaceShadowPass->Apply(pGfxContext.Get(),
        &m_RenderContext,
        sunLight.get(),
        m_GBuffer,
        shadowmapSet,
        m_PostProcessSurfaces);

    pGfxContext->PIXEndEvent();
}
