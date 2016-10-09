#include "pch.h"
#include "Model.h"

#include "Mesh.h"
#include "Camera.h"
#include "RenderContext.h"


Model::Model()
{
}

Model::~Model()
{
}

void Model::DrawPrimitives(RenderContext* pRenderContext, DX12GraphicContext* pGfxContext)
{
	pRenderContext->SetModelMatrix(GetWorldMatrix());
	m_Mesh->DrawPrimitives(pRenderContext, pGfxContext);
}
