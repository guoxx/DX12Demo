#include "pch.h"
#include "Material.h"

#include "../DX12/DX12.h"
#include "RenderContext.h"

#include "Lights/PointLight.h"
#include "Lights/DirectionalLight.h"

#include "../Shaders/CompiledShaders/BaseMaterial.h"
#include "../Shaders/CompiledShaders/BaseMaterial_DepthOnly.h"
#include "../Shaders/CompiledShaders/BaseMaterial_RSM.h"
#include <filesystem>


Material::Material()
{
}

Material::~Material()
{
}

void Material::Load(DX12GraphicsContext* pGfxContext)
{
	//CD3DX12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2DView(D3D12_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM);
	//m_NullDescriptorHandle = DX12GraphicsManager::GetInstance()->RegisterResourceInDescriptorHeap(nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//DX12GraphicsManager::GetInstance()->GetDevice()->CreateShaderResourceView(nullptr, &nullSrvDesc, m_NullDescriptorHandle.GetCpuHandle());

	{
		ShadingConfiguration shadingCfg = ShadingConfiguration_GBuffer;

		DX12RootSignatureDeserializer sigDeserializer{g_BaseMaterial_VS_bytecode};
		m_RootSig[shadingCfg] = sigDeserializer.Deserialize(DX12GraphicsManager::GetInstance()->GetDevice());

		DX12GraphicsPsoDesc psoDesc;
        psoDesc.SetShaderFromBin(DX12ShaderTypeVertex, {g_BaseMaterial_VS, sizeof(g_BaseMaterial_VS)});
        psoDesc.SetShaderFromBin(DX12ShaderTypePixel, {g_BaseMaterial_PS, sizeof(g_BaseMaterial_PS)});
		psoDesc.SetRoogSignature(m_RootSig[shadingCfg].get());
		psoDesc.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT);
		psoDesc.SetDespthStencilFormat(DXGI_FORMAT_D32_FLOAT);
		psoDesc.SetRasterizerState(CD3DX12::RasterizerDefault());

		m_PSO[shadingCfg] = DX12PsoCompiler::Compile(DX12GraphicsManager::GetInstance()->GetDevice(), &psoDesc);
	}

	{
		ShadingConfiguration shadingCfg = ShadingConfiguration_DepthOnly;

        DX12RootSignatureDeserializer sigDeserializer{ g_BaseMaterial_DepthOnly_VS_bytecode };
        m_RootSig[shadingCfg] = sigDeserializer.Deserialize(DX12GraphicsManager::GetInstance()->GetDevice());

		DX12GraphicsPsoDesc psoDesc;
        psoDesc.SetShaderFromBin(DX12ShaderTypeVertex, {g_BaseMaterial_DepthOnly_VS, sizeof(g_BaseMaterial_DepthOnly_VS)});
		psoDesc.SetRoogSignature(m_RootSig[shadingCfg].get());
		psoDesc.SetDespthStencilFormat(DXGI_FORMAT_D32_FLOAT);
		psoDesc.SetRasterizerState(CD3DX12::RasterizerShadow());

		m_PSO[shadingCfg] = DX12PsoCompiler::Compile(DX12GraphicsManager::GetInstance()->GetDevice(), &psoDesc);
	}

	{
		ShadingConfiguration shadingCfg = ShadingConfiguration_RSM;

        DX12RootSignatureDeserializer sigDeserializer{ g_BaseMaterial_RSM_VS_bytecode };
        m_RootSig[shadingCfg] = sigDeserializer.Deserialize(DX12GraphicsManager::GetInstance()->GetDevice());

		DX12GraphicsPsoDesc psoDesc;
        psoDesc.SetShaderFromBin(DX12ShaderTypeVertex, {g_BaseMaterial_RSM_VS, sizeof(g_BaseMaterial_RSM_VS)});
        psoDesc.SetShaderFromBin(DX12ShaderTypePixel, {g_BaseMaterial_RSM_PS, sizeof(g_BaseMaterial_RSM_PS)});
		psoDesc.SetRoogSignature(m_RootSig[shadingCfg].get());
		psoDesc.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM);
		psoDesc.SetDespthStencilFormat(DXGI_FORMAT_D32_FLOAT);
		psoDesc.SetRasterizerState(CD3DX12::RasterizerShadow());

		m_PSO[shadingCfg] = DX12PsoCompiler::Compile(DX12GraphicsManager::GetInstance()->GetDevice(), &psoDesc);
	}

    m_AlbedoMap = LoadTexture(pGfxContext, m_AlbedoMapName, true);
    m_NormalMap = LoadTexture(pGfxContext, m_NormalMapName, false);
    m_RoughnessMap = LoadTexture(pGfxContext, m_RoughnessMapName, false);
    m_MetallicMap = LoadTexture(pGfxContext, m_MetallicMapName, false);
}

std::shared_ptr<DX12Texture> Material::LoadTexture(DX12GraphicsContext* pGfxContext, std::string texname, bool forceSRGB)
{
	std::shared_ptr<DX12Texture> tex = std::shared_ptr<DX12Texture>();
    tex.reset(DX12Texture::LoadFromFile(DX12GraphicsManager::GetInstance()->GetDevice(), pGfxContext, texname.c_str(), forceSRGB));
	return tex;	
}

void Material::Apply(RenderContext* pRenderContext, DX12GraphicsContext* pGfxContext)
{
	ShadingConfiguration shadingCfg = pRenderContext->GetShadingCfg();

	pGfxContext->SetGraphicsRootSignature(m_RootSig[shadingCfg]);

	pGfxContext->SetPipelineState(m_PSO[shadingCfg].get());

	if (shadingCfg == ShadingConfiguration_GBuffer)
	{
		struct View
		{
		    float4x4 mModelViewProj;
		    float4x4 mModelViewProjLastFrame;
		    float4x4 mModel;
		    float4 JitterOffset;
		};

		struct BaseMaterial
		{
			float4 Ambient;
			float4 Diffuse;
			float4 Specular;
			float4 Transmittance;
			float4 Emission;
			float4 Shininess;
			float4 Ior;
			float4 Dissolve;
		};

		View view;

		DirectX::XMMATRIX mModel = pRenderContext->GetModelMatrix();
		DirectX::XMStoreFloat4x4(&view.mModelViewProj, DirectX::XMMatrixTranspose(pRenderContext->GetModelViewProjMatrixWithJitter()));
		DirectX::XMStoreFloat4x4(&view.mModel, DirectX::XMMatrixTranspose(mModel));
        // TODO: camera motion only for now
        DirectX::XMMATRIX mModelViewProjLastFrame = DirectX::XMMatrixMultiply(mModel, pRenderContext->m_mViewProjLastFrame);
		DirectX::XMStoreFloat4x4(&view.mModelViewProjLastFrame, XMMatrixTranspose(mModelViewProjLastFrame));
        DirectX::XMVECTOR jitterOffset = pRenderContext->GetJitterOffset();
        DirectX::XMVECTOR jitterOffsetLastFrame = pRenderContext->m_JitterOffsetLastFrame;
	    view.JitterOffset = DirectX::XMFLOAT4{
	        DirectX::XMVectorGetX(jitterOffset) * 2.0f / pRenderContext->GetScreenWidth(),
	        DirectX::XMVectorGetY(jitterOffset) * 2.0f / pRenderContext->GetScreenHeight(),
	        DirectX::XMVectorGetX(jitterOffsetLastFrame) * 2.0f / pRenderContext->GetScreenWidth(),
	        DirectX::XMVectorGetY(jitterOffsetLastFrame) * 2.0f / pRenderContext->GetScreenHeight(),
	    };

		BaseMaterial baseMaterial;
		//baseMaterial.Ambient = DirectX::XMFLOAT4{ m_Ambient.x, m_Ambient.y, m_Ambient.z, 0.0f };
		//baseMaterial.Diffuse = DirectX::XMFLOAT4{ m_Diffuse.x, m_Diffuse.y, m_Diffuse.z, 0.0f };
		//baseMaterial.Specular = DirectX::XMFLOAT4{ m_Specular.x, m_Specular.y, m_Specular.z, 0.0f };
		//baseMaterial.Transmittance = DirectX::XMFLOAT4{ m_Transmittance.x, m_Transmittance.y, m_Transmittance.z, 0.0f };
		//baseMaterial.Emission = DirectX::XMFLOAT4{ m_Emission.x, m_Emission.y, m_Emission.z, 0.0f };
		//baseMaterial.Shininess = DirectX::XMFLOAT4{ m_Shininess, m_Shininess, m_Shininess, m_Shininess };
		//baseMaterial.Ior = DirectX::XMFLOAT4{ m_Ior, m_Ior, m_Ior, m_Ior };
		//baseMaterial.Dissolve = DirectX::XMFLOAT4{ m_Dissolve, m_Dissolve, m_Dissolve, m_Dissolve };

		pGfxContext->SetGraphicsRootDynamicConstantBufferView(1, &view, sizeof(view));
		pGfxContext->SetGraphicsRootDynamicConstantBufferView(2, &baseMaterial, sizeof(baseMaterial));

		pGfxContext->SetGraphicsDynamicCbvSrvUav(3, 0, m_AlbedoMap->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetGraphicsDynamicCbvSrvUav(3, 1, m_NormalMap->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetGraphicsDynamicCbvSrvUav(3, 2, m_RoughnessMap->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetGraphicsDynamicCbvSrvUav(3, 3, m_MetallicMap->GetStagingSRV().GetCpuHandle());
	}
	else if (shadingCfg == ShadingConfiguration_DepthOnly)
	{
		struct View
		{
			float4x4 mModelViewProj;
		};

		View view;
		DirectX::XMStoreFloat4x4(&view.mModelViewProj, DirectX::XMMatrixTranspose(pRenderContext->GetModelViewProjMatrix()));

		pGfxContext->SetGraphicsRootDynamicConstantBufferView(1, &view, sizeof(view));
	}
	else if (shadingCfg == ShadingConfiguration_RSM)
	{
        // TODO: legacy feature, will work out something better
        assert(false);

		HLSL::BaseMaterialRSMConstants constants;

		DirectX::XMMATRIX mModel = pRenderContext->GetModelMatrix();
		DirectX::XMMATRIX mInverseModel = DirectX::XMMatrixInverse(nullptr, mModel);
		DirectX::XMMATRIX mInverseTransposeModel = DirectX::XMMatrixTranspose(mInverseModel);
		DirectX::XMStoreFloat4x4(&constants.mModelViewProj, DirectX::XMMatrixTranspose(pRenderContext->GetModelViewProjMatrix()));
		DirectX::XMStoreFloat4x4(&constants.mInverseTransposeModel, mInverseTransposeModel);

		const ILight* pLightForRSM = pRenderContext->GetCurrentLightForRSM();
		const DirectionalLight* pDirLight = dynamic_cast<const DirectionalLight*>(pLightForRSM);
		if (pDirLight != nullptr)
		{
			constants.LightType = 0;
			constants.DirectionalLightDirection = pDirLight->GetDirection();
			constants.DirectionalLightIrradiance = pDirLight->GetIrradiance();
		}
		else
		{
			const PointLight* pPointLight = dynamic_cast<const PointLight*>(pLightForRSM);
			assert(pPointLight != nullptr);

			constants.LightType = 1;
			DirectX::XMStoreFloat4(&constants.PointLightPosition, pPointLight->GetTranslation());
			DirectX::XMStoreFloat4(&constants.PointLightIntensity, pPointLight->GetRadiantPower());
		}

		pGfxContext->SetGraphicsRootDynamicConstantBufferView(1, &constants, sizeof(constants));

		pGfxContext->SetGraphicsRootDescriptorTable(2, m_AlbedoMap->GetSRV());
	}
	else
	{
		assert(false);
	}
}
