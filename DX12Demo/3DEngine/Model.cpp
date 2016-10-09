#include "pch.h"
#include "Model.h"

#include "Mesh.h"
#include "Primitive.h"
#include "Material.h"
#include "Camera.h"


Model::Model()
{
}

Model::~Model()
{
}

void Model::DrawPrimitives(const Camera* pCamera, DX12GraphicContext* pGfxContext)
{
	for (auto prim : m_Mesh->m_Primitives)
	{
		prim->m_Material->Apply(pGfxContext);

		pGfxContext->SetGraphicsRootStructuredBuffer(0, m_Mesh->m_VertexBuffer.get());
		pGfxContext->IASetIndexBuffer(m_Mesh->m_IndexBuffer.get());
		pGfxContext->DrawIndexed(prim->m_IndexCount, prim->m_StartIndexLocation);
	}
}
