#include "pch.h"
#include "Material.h"

#include "../DX12/DX12.h"
#include "RenderContext.h"

namespace
{
	ConstantBuffer(View)
	{
		float4x4 mModelViewProj;
		float4x4 mInverseTransposeModel;
	};

	ConstantBuffer(BaseMaterial)
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
}

Material::Material()
{
}

Material::~Material()
{
}

void Material::Load(DX12GraphicContext* pGfxContext)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
	nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	nullSrvDesc.Texture2D.MipLevels = 1;
	nullSrvDesc.Texture2D.MostDetailedMip = 0;
	nullSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	m_NullDescriptorHandle = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	DX12GraphicManager::GetInstance()->GetDevice()->CreateShaderResourceView(nullptr, &nullSrvDesc, m_NullDescriptorHandle.GetCpuHandle());

	D3D12_DESCRIPTOR_RANGE descriptorRanges[] = {
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
	};


	DX12RootSignatureCompiler sigCompiler;
	sigCompiler.Begin(4, 1);
	sigCompiler.End();
	sigCompiler[0].InitAsConstantBufferView(0);
	sigCompiler[1].InitAsConstantBufferView(1);
	sigCompiler[2].InitAsShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	sigCompiler[3].InitAsDescriptorTable(_countof(descriptorRanges), descriptorRanges, D3D12_SHADER_VISIBILITY_PIXEL);
	CD3DX12_STATIC_SAMPLER_DESC staticSampDesc = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT);
	staticSampDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	sigCompiler.InitStaticSampler(staticSampDesc);
	m_RootSig = sigCompiler.Compile(DX12GraphicManager::GetInstance()->GetDevice());

	DX12GraphicPsoCompiler psoCompiler;
	psoCompiler.SetShaderFromFile(DX12ShaderTypeVertex, L"BaseMaterial.hlsl", "VSMain");
	psoCompiler.SetShaderFromFile(DX12ShaderTypePixel, L"BaseMaterial.hlsl", "PSMain");
	psoCompiler.SetRoogSignature(m_RootSig.get());
	psoCompiler.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM);
	psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_D32_FLOAT);

	CD3DX12_RASTERIZER_DESC rasterizerDesc{ D3D12_DEFAULT };
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	psoCompiler.SetRasterizerState(rasterizerDesc);

	m_PSO = psoCompiler.Compile(DX12GraphicManager::GetInstance()->GetDevice());

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
	pGfxContext->SetGraphicsRootSignature(m_RootSig);

	pGfxContext->SetPipelineState(m_PSO.get());

	DirectX::XMMATRIX mModel = pRenderContext->GetModelMatrix();
	DirectX::XMMATRIX mInverseModel = DirectX::XMMatrixInverse(nullptr, mModel);
	DirectX::XMMATRIX mInverseTransposeModel = DirectX::XMMatrixTranspose(mInverseModel);
	View view;
	DirectX::XMStoreFloat4x4(&view.mModelViewProj, DirectX::XMMatrixTranspose(pRenderContext->GetModelViewProjMatrix()));
	DirectX::XMStoreFloat4x4(&view.mInverseTransposeModel, mInverseTransposeModel);

	BaseMaterial baseMaterial;
	baseMaterial.Ambient = DirectX::XMFLOAT4{ m_Ambient.x, m_Ambient.y, m_Ambient.z, 0.0f };
	baseMaterial.Diffuse = DirectX::XMFLOAT4{ m_Diffuse.x, m_Diffuse.y, m_Diffuse.z, 0.0f };
	baseMaterial.Specular = DirectX::XMFLOAT4{ m_Specular.x, m_Specular.y, m_Specular.z, 0.0f };
	baseMaterial.Transmittance = DirectX::XMFLOAT4{ m_Transmittance.x, m_Transmittance.y, m_Transmittance.z, 0.0f };
	baseMaterial.Emission = DirectX::XMFLOAT4{ m_Emission.x, m_Emission.y, m_Emission.z, 0.0f };
	baseMaterial.Shininess = DirectX::XMFLOAT4{ m_Shininess, m_Shininess, m_Shininess, m_Shininess };
	baseMaterial.Ior = DirectX::XMFLOAT4{ m_Ior, m_Ior, m_Ior, m_Ior };
	baseMaterial.Dissolve = DirectX::XMFLOAT4{ m_Dissolve, m_Dissolve, m_Dissolve, m_Dissolve };

	pGfxContext->SetGraphicsRootDynamicConstantBufferView(0, &view, sizeof(view));
	pGfxContext->SetGraphicsRootDynamicConstantBufferView(1, &baseMaterial, sizeof(baseMaterial));

	if (m_DiffuseTexture.get() == nullptr)
	{
		pGfxContext->SetGraphicsRootDescriptorTable(3, m_NullDescriptorHandle);
	}
	else
	{
		pGfxContext->SetGraphicsRootDescriptorTable(3, m_DiffuseTexture->GetSRV());
	}
}
