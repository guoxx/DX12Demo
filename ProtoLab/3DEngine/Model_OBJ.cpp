#include "pch.h"
#include "Model.h"

#include "Mesh.h"
#include "Material.h"
#include "Primitive.h"

#include "MaterialManager.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <filesystem>


std::vector<std::shared_ptr<Model>> Model::LoadFromFile(DX12Device* device, DX12GraphicsContext* pGfxContext, const char* filename)
{
    std::experimental::filesystem::path fullpath = std::experimental::filesystem::current_path();
    fullpath.append(filename);
    std::experimental::filesystem::path directory = fullpath.parent_path();

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
            pIdxData[faceIdx * 3 + 0] = mesh->mFaces[faceIdx].mIndices[0];
            pIdxData[faceIdx * 3 + 1] = mesh->mFaces[faceIdx].mIndices[1];
            pIdxData[faceIdx * 3 + 2] = mesh->mFaces[faceIdx].mIndices[2];
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

            aiString diffuseTexPath;
            aiString normalMapPath;
            aiString roughnessMapPath;
            aiString metallicMapPath;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexPath) == aiReturn_SUCCESS)
            {
                auto filepath = directory;
                filepath /= diffuseTexPath.C_Str();
                prim->m_Material->m_DiffuseMapName = filepath.string();
            }
            else
            {
                prim->m_Material->m_DiffuseMapName = std::experimental::filesystem::current_path().append("Textures/Default.dds").string();
            }

            if (material->GetTexture(aiTextureType_NORMALS, 0, &normalMapPath) == aiReturn_SUCCESS ||
                material->GetTexture(aiTextureType_HEIGHT, 0, &normalMapPath) == aiReturn_SUCCESS)
            {
                auto filepath = directory;
                filepath /= normalMapPath.C_Str();
                prim->m_Material->m_NormalMapName = filepath.string();
            }
            else
            {
                prim->m_Material->m_NormalMapName = std::experimental::filesystem::current_path().append("Textures/DefaultNormalMap.dds").string();
            }

            if (material->GetTexture(aiTextureType_SHININESS, 0, &roughnessMapPath) == aiReturn_SUCCESS)
            {
                auto filepath = directory;
                filepath /= roughnessMapPath.C_Str();
                prim->m_Material->m_RoughnessMapName = filepath.string();
            }
            else
            {
                prim->m_Material->m_RoughnessMapName = std::experimental::filesystem::current_path().append("Textures/DefaultRoughness.dds").string();
            }

            if (material->GetTexture(aiTextureType_AMBIENT, 0, &metallicMapPath) == aiReturn_SUCCESS)
            {
                auto filepath = directory;
                filepath /= metallicMapPath.C_Str();
                prim->m_Material->m_MetallicMapName = filepath.string();
            }
            else
            {
                prim->m_Material->m_MetallicMapName = std::experimental::filesystem::current_path().append("Textures/DefaultBlack.dds").string();
            }

            prim->m_Material->Load(pGfxContext);
        }
    }

    return models;
}
