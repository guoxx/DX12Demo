#pragma once

#include "Actor.h"

#include "../DX12/DX12.h"

class Mesh;
class Camera;
class Material;
class RenderContext;

class Model : public Actor
{
public:
	Model();
	~Model();

	static std::vector<std::shared_ptr<Model>> LoadOBJ(DX12Device* device, DX12GraphicsContext* pGfxContext, const char* objFilename, const char* mtlBasepath);

	void DrawPrimitives(RenderContext* pRenderContext, DX12GraphicsContext* pGfxContext);

private:
	static void Model::_LoadMTLMaterial(void* materialData, Material* pMaterial, DX12GraphicsContext* pGfxContext);

	std::string m_Name;
	std::wstring m_wName;

	std::shared_ptr<Mesh> m_Mesh;
};

