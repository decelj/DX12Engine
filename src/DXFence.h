#pragma once

#include "stdafx.h"
#include <d3d12.h>

class DXFence
{
public:
	DXFence();
	~DXFence();

	bool Ready() const;
	void Signal();
	void Wait();

private:
	uint32_t						m_Value;
	ReleasedUniquePtr<ID3D12Fence>	m_Fence;
	HANDLE							m_Event;
};
