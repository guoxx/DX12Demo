#include "pch.h"
#include "DX12GraphicManager.h"

#include "DX12Device.h"

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
#if defined(_DEBUG)
    // Enable the D3D12 debug layer.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_GRAPHICS_PPV_ARGS(debugController.GetAddressOf()))))
        {
            debugController->EnableDebugLayer();
        }
    }
#endif

	m_Device = std::make_unique<DX12Device>();
}

DX12GraphicManager::~DX12GraphicManager()
{
}

void DX12GraphicManager::CreateGraphicCommandQueues(uint32_t cnt)
{
	for (uint32_t i = 0; i < cnt; ++i)
	{
		m_GraphicQueues.push_back(ComPtr<ID3D12CommandQueue>{ m_Device->CreateGraphicCommandQueue(0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0) });
	}
}

std::shared_ptr<DX12GraphicContext> DX12GraphicManager::CreateGraphicContext()
{
	return nullptr;
}

void DX12GraphicManager::ExecuteGraphicContext(DX12GraphicContext* ctx)
{
}

void DX12GraphicManager::RegisterSRVInDescriptorHeap(ID3D12Resource * resource)
{
}

void DX12GraphicManager::RegisterRTVInDescriptorheap(ID3D12Resource * resource)
{
}

void DX12GraphicManager::RegisterDSVInDescriptorHeap(ID3D12Resource * resource)
{
}
