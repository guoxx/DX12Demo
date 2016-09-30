#pragma once

#include "../DX12/DX12.h"

class Mesh
{
public:
	Mesh();
	~Mesh();

private:
	std::shared_ptr<DX12StructuredBuffer> m_VertexBuffer;
	std::shared_ptr<DX12IndexBuffer> m_IndexBuffer;
};

