#pragma once

#include "stdafx.h"
#include <d3d12.h>

class DXDevice;

class DXFence
{
public:
	DXFence(DXDevice& device);
	~DXFence();

	void Signal(DXDevice& device);
	void Wait();

private:
	uint32_t						m_Value;
	ReleasedUniquePtr<ID3D12Fence>	m_Fence;
	HANDLE							m_Event;
};
