#include "pch.h"
#include "DX12GraphicManager.h"

#include "DX12Device.h"
#include "DX12DescriptorManager.h"
#include "DX12GraphicContext.h"
#include "DX12Fence.h"

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

	m_DescriptorManager = std::make_unique<DX12DescriptorManager>(m_Device.get());

	m_FenceManager = std::make_unique<DX12FenceManager>(m_Device.get());

	m_GraphicContextIdx = 0;
	for (uint32_t i = 0; i < m_GraphicContexts.size(); ++i)
	{
		m_GraphicContexts[i] = std::make_shared<DX12GraphicContext>(m_Device.get());
	}
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

DX12GraphicContext* DX12GraphicManager::BegineGraphicContext()
{
	uint32_t idx = m_GraphicContextIdx % DX12NumGraphicContexts;
	++m_GraphicContextIdx;

	std::shared_ptr<DX12GraphicContext> ctx = m_GraphicContexts[idx];
	// TODO
	assert(false);
	//ctx->Reset();
	return ctx.get();
}

void DX12GraphicManager::EndGraphicContext(DX12GraphicContext * ctx)
{
	ctx->Close();
}

void DX12GraphicManager::ExecuteGraphicContext(DX12GraphicContext* ctx)
{
	ID3D12CommandList* cmdLists[] = { ctx->GetCommandList(), };
	m_GraphicQueues[0]->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	ctx->ClearState();
	// TODO
	assert(false);
	//ctx->SignalFence(m_GraphicQueues[0].Get());
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12GraphicManager::RegisterResourceInDescriptorHeap(ID3D12Resource * resource, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	return D3D12_CPU_DESCRIPTOR_HANDLE();
}

