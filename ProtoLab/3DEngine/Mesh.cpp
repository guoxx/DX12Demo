#include "pch.h"
#include "Mesh.h"

#include "Camera.h"
#include "Primitive.h"
#include "Material.h"
#include "RenderContext.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::DrawPrimitives(RenderContext* pRenderContext, DX12GraphicContext* pGfxContext)
{
	for (auto prim : m_Primitives)
	{
		prim->m_Material->Apply(pRenderContext, pGfxContext);

		pGfxContext->SetGraphicsRootStructuredBuffer(0, m_VertexBuffer.get());
		pGfxContext->IASetIndexBuffer(m_IndexBuffer.get());
		pGfxContext->DrawIndexed(prim->m_IndexCount, prim->m_StartIndexLocation);
	}
}