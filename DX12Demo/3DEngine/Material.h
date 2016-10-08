#pragma once

class Model;
class DX12Texture;
class DX12GraphicContext;

class Material
{
	friend class Model;

public:
	Material();
	~Material();

	void LoadTextures(DX12GraphicContext* pGfxContext);

private:
	std::shared_ptr<DX12Texture> LoadTexture(DX12GraphicContext* pGfxContext, std::string texname);

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

	int m_Dummy; // Suppress padding warning.

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
};
