#include "pch.h"
#include "Model.h"

#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


Model::Model()
{
}

Model::~Model()
{
}

std::vector<std::shared_ptr<Model>> Model::LoadOBJ(DX12Device* device, DX12GraphicContext* pGfxContext, const char* objFilename, const char* mtlBasepath)
{
	std::vector<std::shared_ptr<Model>> models;

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	LoadObj(shapes, materials, err, objFilename, mtlBasepath, tinyobj::triangulation | tinyobj::calculate_normals);
	if (!err.empty())
	{
		DX::Print("%s\n", err.c_str());
	}

	for (auto shape : shapes)
	{
		std::shared_ptr<Model> mod = std::make_shared<Model>();
		mod->m_Name = shape.name;
		models.push_back(mod);

		bool hasNormal = false;
		bool hasTexcoord = false;

		uint64_t numVertex = shape.mesh.positions.size() / 3;

		uint64_t vertexDataSizeInBytes = shape.mesh.positions.size() * sizeof(float);
		uint64_t vertexDataStrideInBytes = sizeof(float) * 3;

		if (shape.mesh.normals.size() > 0)
		{
			hasNormal = true;

			vertexDataSizeInBytes += shape.mesh.normals.size() * sizeof(float);
			vertexDataStrideInBytes = sizeof(float) * 3;
			assert(numVertex == shape.mesh.normals.size()/3);
		}

		if (shape.mesh.texcoords.size() > 0)
		{
			hasTexcoord = true;
			vertexDataSizeInBytes += shape.mesh.texcoords.size() * sizeof(float);
			vertexDataStrideInBytes = sizeof(float) * 2;
			assert(numVertex == shape.mesh.texcoords.size()/2);
		}

		uint64_t indexDataSizeInBytes = shape.mesh.indices.size() * sizeof(uint32_t);

		uint8_t* pVertexData = new uint8_t[vertexDataSizeInBytes];
		uint32_t* pIndexData = shape.mesh.indices.data();

		for (uint64_t i = 0; i < numVertex; ++i)
		{
			float* pData = reinterpret_cast<float*>(pVertexData + vertexDataStrideInBytes * i);

			pData[0] = shape.mesh.positions[i * 3 + 0];
			pData[1] = shape.mesh.positions[i * 3 + 1];
			pData[2] = shape.mesh.positions[i * 3 + 2];

			if (hasNormal)
			{
				pData[3] = shape.mesh.normals[i * 3 + 0];
				pData[4] = shape.mesh.normals[i * 3 + 1];
				pData[5] = shape.mesh.normals[i * 3 + 2];
			}

			if (hasTexcoord)
			{
				pData[6] = shape.mesh.texcoords[i * 2 + 0];
				pData[7] = shape.mesh.texcoords[i * 2 + 1];
			}
		}

		mod->m_Mesh->m_VertexBuffer = std::make_shared<DX12StructuredBuffer>(device, vertexDataSizeInBytes, 0, vertexDataStrideInBytes);
		mod->m_Mesh->m_IndexBuffer = std::make_shared<DX12IndexBuffer>(device, indexDataSizeInBytes, 0, DXGI_FORMAT_R32_UINT);

		DX12GraphicManager::GetInstance()->UpdateBufer(pGfxContext, mod->m_Mesh->m_VertexBuffer.get(), pVertexData, vertexDataSizeInBytes);
		DX12GraphicManager::GetInstance()->UpdateBufer(pGfxContext, mod->m_Mesh->m_IndexBuffer.get(), pIndexData, indexDataSizeInBytes);

		delete [] pVertexData;
	}

	return models;
}
