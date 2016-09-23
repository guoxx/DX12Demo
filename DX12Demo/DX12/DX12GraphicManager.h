#pragma once

#include "DX12Device.h"

class DX12GraphicManager
{
public:
	static void Initialize();
	static void Finalize();

	static DX12GraphicManager* GetInstance() { return s_GfxManager; }
	static DX12Device* GetDevice() { return s_GfxManager->m_Device.get(); }

private:
	DX12GraphicManager();
	~DX12GraphicManager();

	std::unique_ptr<DX12Device> m_Device;

private:
	static DX12GraphicManager* s_GfxManager;
};

