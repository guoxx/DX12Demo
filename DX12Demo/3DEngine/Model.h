#pragma once

#include "Actor.h"

#include "../DX12/DX12.h"

class Mesh;
class Material;

class Model : public Actor
{
public:
	Model();
	~Model();

	static std::vector<std::shared_ptr<Model>> LoadOBJ(DX12Device* device, DX12GraphicContext* pGfxContext, const char* objFilename, const char* mtlBasepath);

private:
	std::string m_Name;
	std::shared_ptr<Mesh> m_Mesh;
	std::shared_ptr<Material> m_Material;
};

