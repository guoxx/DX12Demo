#pragma once

#include "GraphicsEngineDefinition.h"
#include "../DX12/DX12.h"

class Model;
class RenderContext;

class Material
{
	friend class Model;

public:
	Material();
	~Material();

	void Load(DX12GraphicsContext* pGfxContext);

	void Apply(RenderContext* pRenderContext, DX12GraphicsContext* pGfxContext);

private:
	std::shared_ptr<DX12Texture> LoadTexture(DX12GraphicsContext* pGfxContext, std::string texname, bool forceSRGB);

	std::string m_Name;

    std::string m_DiffuseMapName;
    std::string m_NormalMapName;
    std::string m_RoughnessMapName;
    std::string m_MetallicMapName;

    std::shared_ptr<DX12Texture> m_DiffuseMap;
    std::shared_ptr<DX12Texture> m_NormalMap;
    std::shared_ptr<DX12Texture> m_RoughnessMap;
    std::shared_ptr<DX12Texture> m_MetallicMap;

	std::shared_ptr<DX12RootSignature> m_RootSig[ShadingConfiguration_Max];
	std::shared_ptr<DX12PipelineState> m_PSO[ShadingConfiguration_Max];
};
