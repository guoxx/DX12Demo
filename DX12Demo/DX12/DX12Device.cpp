#include "pch.h"
#include "DX12Device.h"


DX12Device* DX12Device::s_Device = nullptr;

void DX12Device::Initialize()
{
	s_Device = new DX12Device();
}

void DX12Device::Finalize()
{
	delete s_Device;
	s_Device = nullptr;
}

DX12Device::DX12Device()
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

    DX::ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_GRAPHICS_PPV_ARGS(m_d3dDevice.ReleaseAndGetAddressOf())));
}

DX12Device::~DX12Device()
{
}
