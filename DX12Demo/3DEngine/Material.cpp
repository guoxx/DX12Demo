#include "pch.h"
#include "Material.h"

#include "../DX12/DX12.h"
#include "RenderContext.h"

namespace
{
	ConstantBuffer(View)
	{
		float4x4 mModelViewProj;
	};

	ConstantBuffer(BaseMaterial)
	{
		float3 Ambient;
		float3 Diffuse;
		float3 Specular;
		float3 Transmittance;
		float3 Emission;
		float Shininess;
		float Ior;
		float Dissolve;
		int Illum;
	};
}

Material::Material()
{
}

Material::~Material()
{
}

void Material::Load(DX12GraphicContext* pGfxContext)
{
	DX12RootSignatureCompiler sigCompiler;
	sigCompiler.Begin(3, 0);
	sigCompiler.End();
	sigCompiler[0].InitAsConstantBufferView(0);
	sigCompiler[1].InitAsConstantBufferView(1);
	sigCompiler[2].InitAsShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_RootSig = sigCompiler.Compile(DX12GraphicManager::GetInstance()->GetDevice());

	DX12GraphicPsoCompiler psoCompiler;
	psoCompiler.SetShaderFromFile(DX12ShaderTypeVertex, L"BaseMaterial.hlsl", "VSMain");
	psoCompiler.SetShaderFromFile(DX12ShaderTypePixel, L"BaseMaterial.hlsl", "PSMain");
	psoCompiler.SetRoogSignature(m_RootSig.get());
	psoCompiler.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_D32_FLOAT);

	CD3DX12_RASTERIZER_DESC rasterizerDesc{ D3D12_DEFAULT };
	rasterizerDesc.FrontCounterClockwise = true;
	psoCompiler.SetRasterizerState(rasterizerDesc);

	m_PSO = psoCompiler.Compile(DX12GraphicManager::GetInstance()->GetDevice());

	m_ViewConstantsBuffer = std::make_shared<DX12ConstantsBuffer>(DX12GraphicManager::GetInstance()->GetDevice(), sizeof(View), 0);
	m_MaterialConstantsBuffer = std::make_shared<DX12ConstantsBuffer>(DX12GraphicManager::GetInstance()->GetDevice(), sizeof(BaseMaterial), 0);

	if (!m_AmbientTexName.empty())
	{
		m_AmbientTexture = LoadTexture(pGfxContext, m_AmbientTexName);
	}
	if (!m_DiffuseTexName.empty())
	{
		m_DiffuseTexture = LoadTexture(pGfxContext, m_DiffuseTexName);
	}
	if (!m_SpecularTexName.empty())
	{
		m_SpecularTexture = LoadTexture(pGfxContext, m_SpecularTexName);
	}
	if (!m_SpecularHighlightTexName.empty())
	{
		m_SpecularHighlightTexture = LoadTexture(pGfxContext, m_SpecularHighlightTexName);
	}
	if (!m_BumpTexName.empty())
	{
		m_BumpTexture = LoadTexture(pGfxContext, m_BumpTexName);
	}
	if (!m_DisplacementTexName.empty())
	{
		m_DisplacementTexture = LoadTexture(pGfxContext, m_DisplacementTexName);
	}
	if (!m_AlphaTexName.empty())
	{
		m_AlphaTexture = LoadTexture(pGfxContext, m_AlphaTexName);
	}
}

std::shared_ptr<DX12Texture> Material::LoadTexture(DX12GraphicContext* pGfxContext, std::string texname)
{
	std::shared_ptr<DX12Texture> tex = std::shared_ptr<DX12Texture>();
	tex.reset(DX12Texture::LoadTGAFromFile(DX12GraphicManager::GetInstance()->GetDevice(), pGfxContext, texname.c_str()));
	return tex;	
}

void Material::Apply(RenderContext* pRenderContext, DX12GraphicContext* pGfxContext)
{
	pGfxContext->SetGraphicsRootSignature(m_RootSig.get());

	pGfxContext->SetPipelineState(m_PSO.get());

	View view;
	DirectX::XMStoreFloat4x4(&view.mModelViewProj, DirectX::XMMatrixTranspose(pRenderContext->GetModelViewProjMatrix()));

	BaseMaterial baseMaterial;
	baseMaterial.Diffuse = m_Diffuse;

	pGfxContext->ResourceTransitionBarrier(m_ViewConstantsBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
	DX12GraphicManager::GetInstance()->UpdateBufer(pGfxContext, m_ViewConstantsBuffer.get(), &view, sizeof(view));
	pGfxContext->ResourceTransitionBarrier(m_ViewConstantsBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	pGfxContext->ResourceTransitionBarrier(m_MaterialConstantsBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
	DX12GraphicManager::GetInstance()->UpdateBufer(pGfxContext, m_MaterialConstantsBuffer.get(), &baseMaterial, sizeof(baseMaterial));
	pGfxContext->ResourceTransitionBarrier(m_MaterialConstantsBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	pGfxContext->SetGraphicsRootConstantBufferView(0, m_ViewConstantsBuffer.get());
	pGfxContext->SetGraphicsRootConstantBufferView(1, m_MaterialConstantsBuffer.get());
}
