#pragma once

class DX12Device;

class DX12ShaderCompiler
{
    static ComPtr<ID3DBlob> CompileFromFile(const wchar_t* file, const char* entry, const char* profile);
};