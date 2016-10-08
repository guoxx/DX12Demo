#pragma once

#include "../DX12/DX12.h"

class Model;
class Primitive;

class Mesh
{
	friend class Model;

public:
	Mesh();
	~Mesh();

private:
	std::shared_ptr<DX12StructuredBuffer> m_VertexBuffer;
	std::shared_ptr<DX12IndexBuffer> m_IndexBuffer;
	std::vector<std::shared_ptr<Primitive>> m_Primitives;
};

