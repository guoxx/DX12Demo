#pragma once

#include "3DEngineDefinition.h"
#include "../DX12/DX12.h"

class Model;
class RenderContext;

class Material
{
	friend class Model;

public:
	Material();
	~Material();

	void Load(DX12GraphicContext* pGfxContext);

	void Apply(RenderContext* pRenderContext, DX12GraphicContext* pGfxContext);

private:
	std::shared_ptr<DX12Texture> LoadTexture(DX12GraphicContext* pGfxContext, std::string texname, bool sRGB);

	std::string m_Name;

	DirectX::XMFLOAT3 m_Ambient;
	DirectX::XMFLOAT3 m_Diffuse;
	DirectX::XMFLOAT3 m_Specular;
	DirectX::XMFLOAT3 m_Transmittance;
	DirectX::XMFLOAT3 m_Emission;
	float m_Shininess;
	float m_Ior;      // index of refraction
	float m_Dissolve; // 1 == opaque; 0 == fully transparent
	// illumination model (see http://www.fileformat.info/format/material/)
	int m_Illum;

	std::string m_AmbientTexName;            // map_Ka
	std::string m_DiffuseTexName;            // map_Kd
	std::string m_SpecularTexName;           // map_Ks
	std::string m_SpecularHighlightTexName; // map_Ns
	std::string m_BumpTexName;               // map_bump, bump
	std::string m_DisplacementTexName;       // disp
	std::string m_AlphaTexName;              // map_d
	std::map<std::string, std::string> m_UnknownParameters;

	std::shared_ptr<DX12Texture> m_AmbientTexture;
	std::shared_ptr<DX12Texture> m_DiffuseTexture;
	std::shared_ptr<DX12Texture> m_SpecularTexture;
	std::shared_ptr<DX12Texture> m_SpecularHighlightTexture;
	std::shared_ptr<DX12Texture> m_BumpTexture;
	std::shared_ptr<DX12Texture> m_DisplacementTexture;
	std::shared_ptr<DX12Texture> m_AlphaTexture;

	std::shared_ptr<DX12RootSignature> m_RootSig[ShadingConfiguration_Max];
	std::shared_ptr<DX12PipelineState> m_PSO[ShadingConfiguration_Max];

	DX12DescriptorHandle m_NullDescriptorHandle;
};
