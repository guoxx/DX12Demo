//
// pch.h
// Header for standard system include files.
//

#pragma once

// Use the C++ standard templated min/max
#define NOMINMAX

#ifdef __XBOX_ONE__
#include <xdk.h>
#include <d3d12_x.h>
#include <d3dx12_x.h>
#include <d3dcompiler_x.h>
#else
#include <dxgi1_5.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <d3dcompiler.h>
#include "DX12/d3dx12.h"
#define IID_GRAPHICS_PPV_ARGS IID_PPV_ARGS
#endif

#include <wrl.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <algorithm>
#include <memory>
#include <vector>

#include <pix.h>

using Microsoft::WRL::ComPtr;

namespace DX
{
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            // Set a breakpoint on this line to catch DirectX API errors
#ifdef __XBOX_ONE__
            throw Platform::Exception::CreateException(hr);
#else
			throw std::exception();
#endif
        }
    }
}
