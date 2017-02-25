#pragma once

class RingBufferAllocator
{
public:
	RingBufferAllocator(uint64_t ptr, uint64_t sizeInBytes)
		: m_Ptr{ ptr }
		, m_SizeInBytes{ sizeInBytes }
	{
	}

	~RingBufferAllocator()
	{
	}

	uint64_t Alloc(uint64_t sizeInBytes, uint64_t alignInBytes = 4)
	{
		uint64_t offsetWithAlignment = DX::NextMultiple(m_Offset, alignInBytes);
		if (offsetWithAlignment + sizeInBytes >= m_SizeInBytes)
		{
			m_Offset = 0;
			return Alloc(sizeInBytes, alignInBytes);
		}
		else
		{
			uint64_t ptr = m_Ptr + offsetWithAlignment;
			m_Offset = offsetWithAlignment + sizeInBytes;
			return ptr;
		}
	}

	void Free(uint64_t ptr)
	{
		(void)ptr;
	}

private:
	const uint64_t m_Ptr;
	const uint64_t m_SizeInBytes;
	uint64_t m_Offset;
};