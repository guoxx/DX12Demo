#include "pch.h"
#include "Model.h"

#include "Mesh.h"
#include "Camera.h"


Model::Model()
{
}

Model::~Model()
{
}

void Model::DrawPrimitives(const Camera* pCamera, DX12GraphicContext* pGfxContext)
{
	m_Mesh->DrawPrimitives(pCamera, pGfxContext);
}
