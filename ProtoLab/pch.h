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

#include <locale>
#include <codecvt>
#include <algorithm>
#include <string>
#include <memory>
#include <vector>
#include <array>
#include <map>

#define USE_PIX
#include <pix3.h>

#include "Utils/Heaponly.h"
#include "Utils/Noncopyable.h"
#include "Utils/Nonmovable.h"
#include "Utils/Helpers.h"
#include "Utils/Color.h"

#include "Shaders/Inc/HLSLShared.h"


#ifdef _XBOX_ONE
using D3D12_COMMAND_QUEUE_FLAGS = D3D12_COMMAND_QUEUE_FLAG;
using GFX_HWND = IUnknown*;
#else
using GFX_HWND = HWND;
#endif

using Microsoft::WRL::ComPtr;

#ifndef ASSERT
	#ifdef _DEBUG
		#define ASSERT(cond, msg) assert(cond)
	#else
		#define ASSERT(cond, msg) 
	#endif
#endif

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
		char buffer[4096];
		va_list args;
		va_start(args, fmt);
		vsprintf_s(buffer, sizeof(buffer), fmt, args);
		va_end(args);
		OutputDebugStringA(buffer);
	}

	// Assign a name to the object to aid with debugging.
#ifndef NDEBUG
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

	inline bool IsPowerOfTwo(uint64_t n)
	{
		return ((n & (n - 1)) == 0 && (n) != 0);
	}

	inline uint64_t NextMultiple(uint64_t value, uint64_t multiple)
	{
		assert(IsPowerOfTwo(multiple));

		return (value + multiple - 1) & ~(multiple - 1);
	}

	template <typename T> __forceinline T DivideByMultiple( T value, size_t alignment )
	{
		return (T)((value + alignment - 1) / alignment);
	}

	inline std::wstring UTF8StrToUTF16(std::string s)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(s);
	}

	template<typename T> T Clamp(T x, T min, T max)
	{
		return std::min<T>(std::max<T>(min, x), max);
	}
}
