//
// pch.h
// Header for standard system include files.
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS

// Use the C++ standard templated min/max
#define NOMINMAX

#ifdef _XBOX_ONE
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
#include <array>

#include <pix.h>

#ifdef _XBOX_ONE
using D3D12_COMMAND_QUEUE_FLAGS = D3D12_COMMAND_QUEUE_FLAG;
using GFX_WHND = IUnknown*;
#else
using GFX_WHND = HWND;
#endif

using Microsoft::WRL::ComPtr;

namespace DX
{
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            // Set a breakpoint on this line to catch DirectX API errors
#ifdef _XBOX_ONE
            throw Platform::Exception::CreateException(hr);
#else
			throw std::exception();
#endif
        }
    }

	inline void Print(const char* fmt, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsprintf_s(buffer, sizeof(buffer), fmt, args);
		va_end(args);
		OutputDebugStringA(buffer);
	}

	// Assign a name to the object to aid with debugging.
#if defined(_DEBUG)
	inline void SetName(ID3D12Object* pObject, LPCWSTR name)
	{
		pObject->SetName(name);
	}
	inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, UINT index)
	{
		WCHAR fullName[50];
		if (swprintf_s(fullName, L"%s[%u]", name, index) > 0)
		{
			pObject->SetName(fullName);
		}
	}
#else
	inline void SetName(ID3D12Object*, LPCWSTR)
	{
	}
	inline void SetNameIndexed(ID3D12Object*, LPCWSTR, UINT)
	{
	}
#endif
}
