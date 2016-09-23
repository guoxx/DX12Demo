#pragma once

#include "DX12CommandContext.h"

class DX12GraphicContext : public DX12CommandContext
{
public:
	DX12GraphicContext(DX12Device* device);
	~DX12GraphicContext();
};
