#pragma once

#include "../DX12/DX12.h"

class Model;
class Primitive;
class Camera;
class RenderContext;

struct VertexLayout
{
    float3 Position;
    float3 Normal;
    float3 Tangent;
    float3 Bitangent;
    float2 UV0;
};

class Mesh
{
	friend class Model;

public:
	Mesh();
	~Mesh();

	void DrawPrimitives(RenderContext* pRenderContext, DX12GraphicsContext* pGfxContext);	

private:
	std::shared_ptr<DX12StructuredBuffer> m_VertexBuffer;
	std::shared_ptr<DX12IndexBuffer> m_IndexBuffer;
	std::vector<std::shared_ptr<Primitive>> m_Primitives;
};

