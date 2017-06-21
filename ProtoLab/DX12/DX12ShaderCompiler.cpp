#include "pch.h"
#include "DX12ShaderCompiler.h"

ComPtr<ID3DBlob> DX12ShaderCompiler::CompileFromFile(const wchar_t* file, const char* entry, const char* profile)
{
	ComPtr<ID3DBlob> shaderBlob = nullptr;
	ComPtr<ID3DBlob> errBlob = nullptr;

	uint32_t compileFlags = 0;
	// TODO: fail to use those flags on X1
#ifndef _XBOX_ONE
#ifdef _DEBUG
	compileFlags |= (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION);
#endif
#endif
	D3DCompileFromFile(file, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, profile, compileFlags, 0, &shaderBlob, &errBlob);
	if (D3DCompileFromFile(file, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, profile, compileFlags, 0, &shaderBlob, &errBlob) != S_OK)
	{
		char* errMsg = static_cast<char*>(errBlob->GetBufferPointer());
		DX::Print("%s\n", errMsg);
	}
	return shaderBlob;
}