#pragma once

#include "DX12CommandContext.h"

class DX12CopyContext : public DX12CommandContext
{
public:
	DX12CopyContext(DX12Device* device);
	~DX12CopyContext();
};

