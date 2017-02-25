#include "pch.h"
#include "MaterialManager.h"


static MaterialManager* s_MaterialManager;

void MaterialManager::Initialize()
{
	s_MaterialManager = new MaterialManager;
}

void MaterialManager::Finalize()
{
	delete s_MaterialManager;
	s_MaterialManager = nullptr;
}

MaterialManager* MaterialManager::GetInstance()
{
	return s_MaterialManager;
}

MaterialManager::MaterialManager()
{
}

MaterialManager::~MaterialManager()
{
}

std::shared_ptr<Material> MaterialManager::GetMaterialByName(std::string name)
{
	auto materialFound = m_MaterialsMap.find(name);
	if (materialFound != m_MaterialsMap.end())
	{
		return materialFound->second;
	}
	else
	{
		return nullptr;
	}
}

void MaterialManager::SetMaterialByName(std::string name, std::shared_ptr<Material> material)
{
	m_MaterialsMap.insert(std::make_pair(name, material));
}
