#include "pch.h"
#include "Material.h"

#include "../DX12/DX12.h"
#include "RenderContext.h"

#include "Lights/PointLight.h"
#include "Lights/DirectionalLight.h"

#include "../Shaders/CompiledShaders/BaseMaterial.h"
#include "../Shaders/CompiledShaders/BaseMaterial_DepthOnly.h"
#include "../Shaders/CompiledShaders/BaseMaterial_RSM.h"


Material::Material()
{
}

Material::~Material()
{
}

void Material::Load(DX12GraphicsContext* pGfxContext)
{
	CD3DX12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2DView(D3D12_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM);

	m_NullDescriptorHandle = DX12GraphicsManager::GetInstance()->RegisterResourceInDescriptorHeap(nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	DX12GraphicsManager::GetInstance()->GetDevice()->CreateShaderResourceView(nullptr, &nullSrvDesc, m_NullDescriptorHandle.GetCpuHandle());

	{
		ShadingConfiguration shadingCfg = ShadingConfiguration_GBuffer;

		DX12RootSignatureCompiler sigCompiler;
		sigCompiler.Begin(4);
		sigCompiler.End();
		sigCompiler[0].InitAsShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		sigCompiler[1].InitAsConstantBufferView(0);
		sigCompiler[2].InitAsConstantBufferView(1);
		sigCompiler.InitDescriptorTable(3, 1, D3D12_SHADER_VISIBILITY_PIXEL);
		sigCompiler.SetupDescriptorRange(3, 0, CD3DX12_DESCRIPTOR_RANGE{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1});
		m_RootSig[shadingCfg] = sigCompiler.Compile(DX12GraphicsManager::GetInstance()->GetDevice());

		DX12GraphicPsoCompiler psoCompiler;
		psoCompiler.SetShaderFromBin(DX12ShaderTypeVertex, g_BaseMaterial_VS, sizeof(g_BaseMaterial_VS));
		psoCompiler.SetShaderFromBin(DX12ShaderTypePixel, g_BaseMaterial_PS, sizeof(g_BaseMaterial_PS));
		psoCompiler.SetRoogSignature(m_RootSig[shadingCfg].get());
		psoCompiler.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT);
		psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_D32_FLOAT);
		psoCompiler.SetRasterizerState(CD3DX12::RasterizerDefault());

		m_PSO[shadingCfg] = psoCompiler.Compile(DX12GraphicsManager::GetInstance()->GetDevice());
	}

	{
		ShadingConfiguration shadingCfg = ShadingConfiguration_DepthOnly;

		DX12RootSignatureCompiler sigCompiler;
		sigCompiler.Begin(2);
		sigCompiler.End();
		sigCompiler[0].InitAsShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		sigCompiler[1].InitAsConstantBufferView(0);
		m_RootSig[shadingCfg] = sigCompiler.Compile(DX12GraphicsManager::GetInstance()->GetDevice());

		DX12GraphicPsoCompiler psoCompiler;
		psoCompiler.SetShaderFromBin(DX12ShaderTypeVertex, g_BaseMaterial_DepthOnly_VS, sizeof(g_BaseMaterial_DepthOnly_VS));
		psoCompiler.SetRoogSignature(m_RootSig[shadingCfg].get());
		psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_D32_FLOAT);
		psoCompiler.SetRasterizerState(CD3DX12::RasterizerShadow());

		m_PSO[shadingCfg] = psoCompiler.Compile(DX12GraphicsManager::GetInstance()->GetDevice());
	}

	{
		ShadingConfiguration shadingCfg = ShadingConfiguration_RSM;

		DX12RootSignatureCompiler sigCompiler;
		sigCompiler.Begin(3);
		sigCompiler.End();
		sigCompiler[0].InitAsShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		sigCompiler[1].InitAsConstantBufferView(0);
		sigCompiler.InitDescriptorTable(2, 1, D3D12_SHADER_VISIBILITY_PIXEL);
		sigCompiler.SetupDescriptorRange(2, 0, CD3DX12_DESCRIPTOR_RANGE{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1});
		m_RootSig[shadingCfg] = sigCompiler.Compile(DX12GraphicsManager::GetInstance()->GetDevice());

		DX12GraphicPsoCompiler psoCompiler;
		psoCompiler.SetShaderFromBin(DX12ShaderTypeVertex, g_BaseMaterial_RSM_VS, sizeof(g_BaseMaterial_RSM_VS));
		psoCompiler.SetShaderFromBin(DX12ShaderTypePixel, g_BaseMaterial_RSM_PS, sizeof(g_BaseMaterial_RSM_PS));
		psoCompiler.SetRoogSignature(m_RootSig[shadingCfg].get());
		psoCompiler.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM);
		psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_D32_FLOAT);
		psoCompiler.SetRasterizerState(CD3DX12::RasterizerShadow());

		m_PSO[shadingCfg] = psoCompiler.Compile(DX12GraphicsManager::GetInstance()->GetDevice());
	}

	if (!m_AmbientTexName.empty())
	{
		m_AmbientTexture = LoadTexture(pGfxContext, m_AmbientTexName, true);
	}
	if (!m_DiffuseTexName.empty())
	{
		m_DiffuseTexture = LoadTexture(pGfxContext, m_DiffuseTexName, true);
	}
	if (!m_SpecularTexName.empty())
	{
		m_SpecularTexture = LoadTexture(pGfxContext, m_SpecularTexName, true);
	}
	if (!m_SpecularHighlightTexName.empty())
	{
		m_SpecularHighlightTexture = LoadTexture(pGfxContext, m_SpecularHighlightTexName, true);
	}
	if (!m_BumpTexName.empty())
	{
		m_BumpTexture = LoadTexture(pGfxContext, m_BumpTexName, false);
	}
	if (!m_DisplacementTexName.empty())
	{
		m_DisplacementTexture = LoadTexture(pGfxContext, m_DisplacementTexName, false);
	}
	if (!m_AlphaTexName.empty())
	{
		m_AlphaTexture = LoadTexture(pGfxContext, m_AlphaTexName, false);
	}
}

std::shared_ptr<DX12Texture> Material::LoadTexture(DX12GraphicsContext* pGfxContext, std::string texname, bool sRGB)
{
	std::shared_ptr<DX12Texture> tex = std::shared_ptr<DX12Texture>();
	std::string ext = texname.substr(texname.length() - 4, 4);
	if (ext == ".tga")
	{
		tex.reset(DX12Texture::LoadFromTGAFile(DX12GraphicsManager::GetInstance()->GetDevice(), pGfxContext, texname.c_str(), sRGB));
	}
	else
	{
		tex.reset(DX12Texture::LoadFromDDSFile(DX12GraphicsManager::GetInstance()->GetDevice(), pGfxContext, texname.c_str(), sRGB));
	}
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
		    float4x4 mInverseTransposeModel;
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

		DirectX::XMMATRIX mModel = pRenderContext->GetModelMatrix();
		DirectX::XMMATRIX mInverseModel = DirectX::XMMatrixInverse(nullptr, mModel);
		DirectX::XMMATRIX mInverseTransposeModel = DirectX::XMMatrixTranspose(mInverseModel);
		View view;
		DirectX::XMStoreFloat4x4(&view.mModelViewProj, DirectX::XMMatrixTranspose(pRenderContext->GetModelViewProjMatrixWithJitter()));
		DirectX::XMStoreFloat4x4(&view.mInverseTransposeModel, mInverseTransposeModel);
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
		baseMaterial.Ambient = DirectX::XMFLOAT4{ m_Ambient.x, m_Ambient.y, m_Ambient.z, 0.0f };
		baseMaterial.Diffuse = DirectX::XMFLOAT4{ m_Diffuse.x, m_Diffuse.y, m_Diffuse.z, 0.0f };
		baseMaterial.Specular = DirectX::XMFLOAT4{ m_Specular.x, m_Specular.y, m_Specular.z, 0.0f };
		baseMaterial.Transmittance = DirectX::XMFLOAT4{ m_Transmittance.x, m_Transmittance.y, m_Transmittance.z, 0.0f };
		baseMaterial.Emission = DirectX::XMFLOAT4{ m_Emission.x, m_Emission.y, m_Emission.z, 0.0f };
		baseMaterial.Shininess = DirectX::XMFLOAT4{ m_Shininess, m_Shininess, m_Shininess, m_Shininess };
		baseMaterial.Ior = DirectX::XMFLOAT4{ m_Ior, m_Ior, m_Ior, m_Ior };
		baseMaterial.Dissolve = DirectX::XMFLOAT4{ m_Dissolve, m_Dissolve, m_Dissolve, m_Dissolve };

		pGfxContext->SetGraphicsRootDynamicConstantBufferView(1, &view, sizeof(view));
		pGfxContext->SetGraphicsRootDynamicConstantBufferView(2, &baseMaterial, sizeof(baseMaterial));

		if (m_DiffuseTexture.get() == nullptr)
		{
			pGfxContext->SetGraphicsRootDescriptorTable(3, m_NullDescriptorHandle);
		}
		else
		{
			pGfxContext->SetGraphicsRootDescriptorTable(3, m_DiffuseTexture->GetSRV());
		}
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

		if (m_DiffuseTexture.get() == nullptr)
		{
			pGfxContext->SetGraphicsRootDescriptorTable(2, m_NullDescriptorHandle);
		}
		else
		{
			pGfxContext->SetGraphicsRootDescriptorTable(2, m_DiffuseTexture->GetSRV());
		}
	}
	else
	{
		assert(false);
	}
}
