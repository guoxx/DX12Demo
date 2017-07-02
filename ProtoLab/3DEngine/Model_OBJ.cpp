#include "pch.h"
#include "Model.h"

#include "Mesh.h"
#include "Material.h"
#include "Primitive.h"

#include "MaterialManager.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#pragma warning(push)
#pragma warning(disable:4201)
#pragma warning(disable:4706)
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#pragma warning(pop)

void Model::_LoadMTLMaterial(void* materialData, Material* pMaterial, DX12GraphicsContext* pGfxContext)
{
	tinyobj::material_t* pMaterialData = (tinyobj::material_t*)materialData;

	pMaterial->m_Name = pMaterialData->name;
	pMaterial->m_Ambient = DirectX::XMFLOAT3(pMaterialData->ambient[0], pMaterialData->ambient[1], pMaterialData->ambient[2]);
	pMaterial->m_Diffuse = DirectX::XMFLOAT3(pMaterialData->diffuse[0], pMaterialData->diffuse[1], pMaterialData->diffuse[2]);
	pMaterial->m_Specular = DirectX::XMFLOAT3(pMaterialData->specular[0], pMaterialData->specular[1], pMaterialData->specular[2]);
	pMaterial->m_Transmittance = DirectX::XMFLOAT3(pMaterialData->transmittance[0], pMaterialData->transmittance[1], pMaterialData->transmittance[2]);
	pMaterial->m_Emission = DirectX::XMFLOAT3(pMaterialData->emission[0], pMaterialData->emission[1], pMaterialData->emission[2]);
	pMaterial->m_Shininess = pMaterialData->shininess;
	pMaterial->m_Ior = pMaterialData->ior;
	pMaterial->m_Dissolve = pMaterialData->dissolve;
	pMaterial->m_Illum = pMaterialData->illum;
	pMaterial->m_AmbientTexName = pMaterialData->ambient_texname;
	pMaterial->m_DiffuseTexName = pMaterialData->diffuse_texname;
	pMaterial->m_SpecularTexName = pMaterialData->specular_texname;
	pMaterial->m_SpecularHighlightTexName = pMaterialData->specular_highlight_texname;
	pMaterial->m_BumpTexName = pMaterialData->bump_texname;
	pMaterial->m_DisplacementTexName = pMaterialData->displacement_texname;
	pMaterial->m_AlphaTexName = pMaterialData->alpha_texname;
	pMaterial->m_UnknownParameters = pMaterialData->unknown_parameter;
	pMaterial->Load(pGfxContext);
}

std::vector<std::shared_ptr<Model>> Model::LoadFromFile(DX12Device* device, DX12GraphicsContext* pGfxContext, const char* filename)
{
    Assimp::Importer importer;
    uint32_t flags = 0;
    flags |= aiProcess_ConvertToLeftHanded;
    flags |= aiProcessPreset_TargetRealtime_Fast;
    flags |= aiProcess_PreTransformVertices;
    const aiScene* scene = importer.ReadFile(filename, flags);
    if (!scene)
    {
        DX::Print(importer.GetErrorString());
        assert(false);
    }

    assert(scene->mNumMeshes != 0);
    assert(scene->mNumMaterials != 0);
    assert(scene->mRootNode->mNumMeshes == 0);

	std::vector<std::shared_ptr<Model>> models;

    for (uint32_t i = 0; i < scene->mRootNode->mNumChildren; ++i)
    {
        std::shared_ptr<Model> mod = std::make_shared<Model>();
        models.push_back(mod);

        const aiNode* node = scene->mRootNode->mChildren[i];
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[0]];
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        assert(node->mNumMeshes == 1);
        assert(mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE);

        const uint32_t indexCnt = mesh->mNumFaces * 3;

        mod->m_Name = node->mName.C_Str();
		mod->m_wName = DX::UTF8StrToUTF16(mod->m_Name);

        std::unique_ptr<VertexLayout[]> vtxDatas = std::unique_ptr<VertexLayout[]>(new typename std::remove_extent<VertexLayout[]>::type[mesh->mNumVertices]());
        VertexLayout* pVtxData = vtxDatas.get();

        std::unique_ptr<uint32_t[]> idxDatas = std::unique_ptr<uint32_t[]>(new typename std::remove_extent<uint32_t[]>::type[indexCnt]());
        uint32_t* pIdxData = idxDatas.get();

        uint32_t sizeOfVtxData = sizeof(VertexLayout) * mesh->mNumVertices;
        uint32_t sizeOfIdxData = sizeof(uint32_t) * indexCnt;

        for (uint32_t vtxIdx = 0; vtxIdx < mesh->mNumVertices; ++vtxIdx)
        {
            pVtxData[vtxIdx].Position.x = mesh->mVertices[vtxIdx].x;
            pVtxData[vtxIdx].Position.y = mesh->mVertices[vtxIdx].y;
            pVtxData[vtxIdx].Position.z = mesh->mVertices[vtxIdx].z;

            pVtxData[vtxIdx].Normal.x = mesh->mNormals[vtxIdx].x;
            pVtxData[vtxIdx].Normal.y = mesh->mNormals[vtxIdx].y;
            pVtxData[vtxIdx].Normal.z = mesh->mNormals[vtxIdx].z;

            pVtxData[vtxIdx].Tangent.x = mesh->mTangents[vtxIdx].x;
            pVtxData[vtxIdx].Tangent.y = mesh->mTangents[vtxIdx].y;
            pVtxData[vtxIdx].Tangent.z = mesh->mTangents[vtxIdx].z;
            
            pVtxData[vtxIdx].Bitangent.x = mesh->mBitangents[vtxIdx].x;
            pVtxData[vtxIdx].Bitangent.y = mesh->mBitangents[vtxIdx].y;
            pVtxData[vtxIdx].Bitangent.z = mesh->mBitangents[vtxIdx].z;

            pVtxData[vtxIdx].UV0.x = mesh->mTextureCoords[0][vtxIdx].x;
            pVtxData[vtxIdx].UV0.y = mesh->mTextureCoords[0][vtxIdx].y;
        }

        for (uint32_t faceIdx = 0; faceIdx < mesh->mNumFaces; ++faceIdx)
        {
            assert(mesh->mFaces[faceIdx].mNumIndices == 3);
            pIdxData[faceIdx + 3 + 0] = mesh->mFaces[faceIdx].mIndices[0];
            pIdxData[faceIdx + 3 + 1] = mesh->mFaces[faceIdx].mIndices[1];
            pIdxData[faceIdx + 3 + 2] = mesh->mFaces[faceIdx].mIndices[2];
        }

        mod->m_Mesh = std::make_shared<Mesh>();
        mod->m_Mesh->m_VertexBuffer = std::make_shared<DX12StructuredBuffer>(device, sizeOfVtxData, 0, sizeof(VertexLayout), DX12GpuResourceUsage_GpuReadOnly);
        mod->m_Mesh->m_IndexBuffer = std::make_shared<DX12IndexBuffer>(device, sizeOfIdxData, 0, DXGI_FORMAT_R32_UINT);

        pGfxContext->ResourceTransitionBarrier(mod->m_Mesh->m_VertexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);
        pGfxContext->ResourceTransitionBarrier(mod->m_Mesh->m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);
        pGfxContext->UploadBuffer(mod->m_Mesh->m_VertexBuffer.get(), pVtxData, sizeOfVtxData);
        pGfxContext->UploadBuffer(mod->m_Mesh->m_IndexBuffer.get(), pIdxData, sizeOfIdxData);
        pGfxContext->ResourceTransitionBarrier(mod->m_Mesh->m_VertexBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
        pGfxContext->ResourceTransitionBarrier(mod->m_Mesh->m_IndexBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

        std::shared_ptr<Primitive> prim = std::make_shared<Primitive>();
        mod->m_Mesh->m_Primitives.push_back(prim);

        prim->m_Mesh = mod->m_Mesh;
        prim->m_StartIndexLocation = 0;
        prim->m_IndexCount = indexCnt;

        aiString materialName;
        aiGetMaterialString(material, AI_MATKEY_NAME, &materialName);
        prim->m_Material = MaterialManager::GetInstance()->GetMaterialByName(materialName.C_Str());
        if (prim->m_Material.get() == nullptr)
        {
            prim->m_Material = std::make_shared<Material>();
            prim->m_Material->m_Name = materialName.C_Str();
            MaterialManager::GetInstance()->SetMaterialByName(materialName.C_Str(), prim->m_Material);
        }
    }

    return models;
}

std::vector<std::shared_ptr<Model>> Model::LoadOBJ(DX12Device* device, DX12GraphicsContext* pGfxContext, const char* objFilename, const char* mtlBasepath)
{
	std::vector<std::shared_ptr<Model>> models;

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	LoadObj(shapes, materials, err, objFilename, mtlBasepath);
	if (!err.empty())
	{
		DX::Print("%s\n", err.c_str());
	}

	for (auto shape : shapes)
	{
		std::shared_ptr<Model> mod = std::make_shared<Model>();
		mod->m_Name = shape.name;
		mod->m_wName = DX::UTF8StrToUTF16(shape.name);
		mod->m_Mesh = std::make_shared<Mesh>();
		models.push_back(mod);

		{
			// load buffers
			bool hasNormal = false;
			bool hasTexcoord = false;

			uint64_t numVertex = shape.mesh.positions.size() / 3;

			uint64_t vertexDataSizeInBytes = numVertex * sizeof(float) * 3;
			uint64_t vertexDataStrideInBytes = sizeof(float) * 3;

			if (shape.mesh.normals.size() > 0)
			{
				hasNormal = true;
				assert(numVertex == shape.mesh.normals.size() / 3);
			}
			assert(hasNormal);
			vertexDataSizeInBytes += numVertex * sizeof(float) * 3;
			vertexDataStrideInBytes += sizeof(float) * 3;

			if (shape.mesh.texcoords.size() > 0)
			{
				hasTexcoord = true;
				assert(numVertex == shape.mesh.texcoords.size() / 2);
			}
			vertexDataSizeInBytes += numVertex * sizeof(float) * 2;
			vertexDataStrideInBytes += sizeof(float) * 2;

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
					pData[7] = 1 - shape.mesh.texcoords[i * 2 + 1];
				}
				else
				{
					pData[6] = 0.0f;
					pData[7] = 0.0f;
				}
			}

			mod->m_Mesh->m_VertexBuffer = std::make_shared<DX12StructuredBuffer>(device, vertexDataSizeInBytes, 0, vertexDataStrideInBytes, DX12GpuResourceUsage_GpuReadOnly);
			mod->m_Mesh->m_IndexBuffer = std::make_shared<DX12IndexBuffer>(device, indexDataSizeInBytes, 0, DXGI_FORMAT_R32_UINT);

			pGfxContext->ResourceTransitionBarrier(mod->m_Mesh->m_VertexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);
			pGfxContext->UploadBuffer(mod->m_Mesh->m_VertexBuffer.get(), pVertexData, vertexDataSizeInBytes);
			pGfxContext->ResourceTransitionBarrier(mod->m_Mesh->m_VertexBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

			pGfxContext->ResourceTransitionBarrier(mod->m_Mesh->m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);
			pGfxContext->UploadBuffer(mod->m_Mesh->m_IndexBuffer.get(), pIndexData, indexDataSizeInBytes);
			pGfxContext->ResourceTransitionBarrier(mod->m_Mesh->m_IndexBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

			delete[] pVertexData;
		}

		{
			// load primitives
			int32_t materialId = shape.mesh.material_ids[0];
			int32_t indexCount = 0;

			// insert default primitive
			std::shared_ptr<Primitive> prim = std::make_shared<Primitive>();
			mod->m_Mesh->m_Primitives.push_back(prim);

			prim->m_Mesh = mod->m_Mesh;
			prim->m_StartIndexLocation = 0;

			prim->m_Material = MaterialManager::GetInstance()->GetMaterialByName(materials[materialId].name);
			if (prim->m_Material.get() == nullptr)
			{
				prim->m_Material = std::make_shared<Material>();
				MaterialManager::GetInstance()->SetMaterialByName(materials[materialId].name, prim->m_Material);
				_LoadMTLMaterial(&materials[materialId], prim->m_Material.get(), pGfxContext);
			}

			for (uint32_t i = 0; i < shape.mesh.material_ids.size(); ++i)
			{
				int32_t id = shape.mesh.material_ids[i];
				if (id != materialId)
				{
					materialId = id;

					// finalize a primitive
					prim->m_IndexCount = indexCount;

					// reset index count
					indexCount = 0;

					// add new primitive
					prim = std::make_shared<Primitive>();
					mod->m_Mesh->m_Primitives.push_back(prim);

					prim->m_Mesh = mod->m_Mesh;
					prim->m_StartIndexLocation = i * 3;

					prim->m_Material = MaterialManager::GetInstance()->GetMaterialByName(materials[materialId].name);
					if (prim->m_Material.get() == nullptr)
					{
						prim->m_Material = std::make_shared<Material>();
						MaterialManager::GetInstance()->SetMaterialByName(materials[materialId].name, prim->m_Material);
						_LoadMTLMaterial(&materials[materialId], prim->m_Material.get(), pGfxContext);
					}
				}
				else
				{
					indexCount += 3;
				}
			}

			prim->m_IndexCount = indexCount;

//#ifdef _DEBUG
//			for (int32_t id : shape.mesh.material_ids)
//			{
//				assert(matId == id);
//			}
//#endif
//			tinyobj::material_t& pMaterialData = materials[matId];
//			mod->m_Material->m_Name = pMaterialData->name;
//			mod->m_Material->m_Ambient = DirectX::XMFLOAT3(pMaterialData->ambient[0], pMaterialData->ambient[1], pMaterialData->ambient[2]);
//			mod->m_Material->m_Diffuse = DirectX::XMFLOAT3(pMaterialData->diffuse[0], pMaterialData->diffuse[1], pMaterialData->diffuse[2]);
//			mod->m_Material->m_Specular = DirectX::XMFLOAT3(pMaterialData->specular[0], pMaterialData->specular[1], pMaterialData->specular[2]);
//			mod->m_Material->m_Transmittance = DirectX::XMFLOAT3(pMaterialData->transmittance[0], pMaterialData->transmittance[1], pMaterialData->transmittance[2]);
//			mod->m_Material->m_Emission = DirectX::XMFLOAT3(pMaterialData->emission[0], pMaterialData->emission[1], pMaterialData->emission[2]);
//			mod->m_Material->m_Shininess = pMaterialData->shininess;
//			mod->m_Material->m_Ior = pMaterialData->ior;
//			mod->m_Material->m_Dissolve = pMaterialData->dissolve;
//			mod->m_Material->m_Illum = pMaterialData->illum;
//			mod->m_Material->m_AmbientTexName = pMaterialData->ambient_texname;
//			mod->m_Material->m_DiffuseTexName = pMaterialData->diffuse_texname;
//			mod->m_Material->m_SpecularTexName = pMaterialData->specular_texname;
//			mod->m_Material->m_SpecularHighlightTexName = pMaterialData->specular_highlight_texname;
//			mod->m_Material->m_BumpTexName = pMaterialData->bump_texname;
//			mod->m_Material->m_DisplacementTexName = pMaterialData->displacement_texname;
//			mod->m_Material->m_AlphaTexName = pMaterialData->alpha_texname;
//			mod->m_Material->m_UnknownParameters = pMaterialData->unknown_parameter;
//
//			mod->m_Material->LoadTextures(pGfxContext);
		}
	}

	return models;
}
