#include "pch.h"
#include "DX12CopyContext.h"

#include "DX12Device.h"

DX12CopyContext::DX12CopyContext(DX12Device* device)
	: DX12CommandContext(device)
{
	m_CommandAllocator = device->CreateCopyCommandAllocator();
	m_CommandList = device->CreateCopyCommandList();
}

DX12CopyContext::~DX12CopyContext()
{
}
