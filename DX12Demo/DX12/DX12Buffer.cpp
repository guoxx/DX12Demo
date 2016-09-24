#include "pch.h"
#include "DX12Buffer.h"

#include "DX12Device.h"

DX12Buffer::DX12Buffer(DX12Device* device)
{
}

DX12Buffer::~DX12Buffer()
{
}

DX12ConstantsBuffer::DX12ConstantsBuffer(DX12Device* device)
	: DX12Buffer(device)
{
	m_Buffer = device->CreateBuffer();
}

DX12ConstantsBuffer::~DX12ConstantsBuffer()
{
}
