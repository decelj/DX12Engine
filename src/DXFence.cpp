#include "stdafx.h"
#include "DXDevice.h"
#include "DXFence.h"

DXFence::DXFence(DXDevice& device)
	: m_Value(0u)
	, m_Fence(nullptr)
{
	m_Fence.reset(device.CreateFence());
	m_Event = CreateEvent(nullptr, false, false, nullptr);
	assert(m_Event);
}

DXFence::~DXFence()
{
	Wait();
	CloseHandle(m_Event);
}

void DXFence::Signal(DXDevice& device)
{
	++m_Value;
	device.SignalFence(m_Fence.get(), m_Value);
}

void DXFence::Wait()
{
	if (m_Fence->GetCompletedValue() != m_Value)
	{
		m_Fence->SetEventOnCompletion(m_Value, m_Event);
		WaitForSingleObjectEx(m_Event, INFINITE, FALSE);
	}
}
