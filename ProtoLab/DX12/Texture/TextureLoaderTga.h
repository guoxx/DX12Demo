#pragma once

#include <cstdlib>
#include <cstdint>
#include <sys/types.h>

#include "tga_reader.h"

class TextureLoaderTga {
public:
    TextureLoaderTga()
    : m_Width(0)
    , m_Height(0)
    , m_DataSize(0)
    , m_Data(nullptr)
    {
    }

    ~TextureLoaderTga()
    {
        if (m_Data != NULL)
        {
            tgaFree(m_Data);
        }
    }

	int32_t GetWidth() const
	{
		return m_Width;
	}	

	int32_t GetHeight() const
	{
		return m_Height;
	}

	int32_t GetDataSize() const
	{
		return m_DataSize;
	}

	const uint8_t* GetData() const
	{
		return m_Data;
	}

    bool LoadTga(const char* file)
	{
		FILE *f = fopen(file, "rb");
		if (f == NULL)
		{
			assert(false);
			return false;
		}

		fseek(f, 0, SEEK_END);
		long sz{ ftell(f) };
		fseek(f, 0, SEEK_SET);

		uint8_t* buffer = new uint8_t[sz];
		size_t used = fread(buffer, 1, sz, f);
		assert(used == sz);

		m_Width = tgaGetWidth(buffer);
		m_Height = tgaGetHeight(buffer);
		m_DataSize = m_Width * m_Height * 4;
		m_Data = reinterpret_cast<uint8_t*>(tgaRead(buffer, TGA_READER_ABGR));

		delete[] buffer;
		buffer = nullptr;

		fclose(f);

		return true;
	}

private:
    int32_t m_Width;
    int32_t m_Height;
    int32_t m_DataSize;
    uint8_t *m_Data;
};