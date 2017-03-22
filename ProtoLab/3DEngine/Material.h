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
	std::shared_ptr<DX12Texture> LoadTexture(DX12GraphicsContext* pGfxContext, std::string texname, bool sRGB);

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

    static bool m_PsoInitialized;
	static std::shared_ptr<DX12RootSignature> m_RootSig[ShadingConfiguration_Max];
	static std::shared_ptr<DX12PipelineState> m_PSO[ShadingConfiguration_Max];

	DX12DescriptorHandle m_NullDescriptorHandle;

    int32_t m_DiffuseTexId;

    static bool s_AllTextureHandlesAllocated;
    static int32_t s_AllTextureHandleIdx;
	static DX12DescriptorHandle s_AllTextureHandles;

public:
    static DX12DescriptorHandle GetAllTextures()
    {
        return s_AllTextureHandles;
    }

    static std::shared_ptr<DX12RootSignature> GetRootSignature(ShadingConfiguration conf)
    {
        return m_RootSig[conf];
    }
};
