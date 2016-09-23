#include "pch.h"
#include "DX12GraphicManager.h"

DX12GraphicManager* DX12GraphicManager::s_GfxManager = nullptr;

void DX12GraphicManager::Initialize()
{
	s_GfxManager = new DX12GraphicManager();
}

void DX12GraphicManager::Finalize()
{
	delete s_GfxManager;
	s_GfxManager = nullptr;
}

DX12GraphicManager::DX12GraphicManager()
{
	m_Device = std::make_unique<DX12Device>();
}


DX12GraphicManager::~DX12GraphicManager()
{
}
