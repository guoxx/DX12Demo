#pragma once

class DX12Device;

class DX12Buffer
{
public:
	DX12Buffer(DX12Device* device);
	virtual ~DX12Buffer();
};

