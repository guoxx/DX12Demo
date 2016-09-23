#include "pch.h"
#include "DX12GraphicContext.h"

#include "DX12Device.h"

DX12GraphicContext::DX12GraphicContext(DX12Device* device)
	: DX12CommandContext(device)
{
	m_CommandAllocator = device->CreateGraphicCommandAllocator();
	m_CommandList = device->CreateGraphicCommandList();
}

DX12GraphicContext::~DX12GraphicContext()
{
}
