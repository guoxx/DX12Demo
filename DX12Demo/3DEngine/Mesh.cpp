#include "pch.h"
#include "Mesh.h"

#include "Camera.h"
#include "Primitive.h"
#include "Material.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::DrawPrimitives(const Camera* pCamera, DX12GraphicContext* pGfxContext)
{
	for (auto prim : m_Primitives)
	{
		prim->m_Material->Apply(pGfxContext);

		pGfxContext->SetGraphicsRootStructuredBuffer(1, m_VertexBuffer.get());
		pGfxContext->IASetIndexBuffer(m_IndexBuffer.get());
		pGfxContext->DrawIndexed(prim->m_IndexCount, prim->m_StartIndexLocation);
	}
}