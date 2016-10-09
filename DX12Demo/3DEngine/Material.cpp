#include "pch.h"
#include "Material.h"

#include "../DX12/DX12.h"


Material::Material()
{
}

Material::~Material()
{
}

void Material::LoadTextures(DX12GraphicContext* pGfxContext)
{
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

void Material::Apply(DX12GraphicContext * pGfxContext)
{
	assert(false);
}
