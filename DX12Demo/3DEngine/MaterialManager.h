#pragma once

class Material;

class MaterialManager
{
public:
	MaterialManager();
	~MaterialManager();

	static void Initialize();

	static void Finalize();

	static MaterialManager* GetInstance();

	std::shared_ptr<Material> GetMaterialByName(std::string name);

	void SetMaterialByName(std::string name, std::shared_ptr<Material> material);

private:
	std::map<std::string, std::shared_ptr<Material>> m_MaterialsMap;
};

