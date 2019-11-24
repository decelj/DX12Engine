#include "stdafx.h"
#include "DXDevice.h"
#include "DXFence.h"

DXFence::DXFence()
	: m_Value(0u)
	, m_Fence(nullptr)
{
	m_Fence.reset(DXDevice::Instance().CreateFence());
	m_Event = CreateEvent(nullptr, false, false, nullptr);
	assert(m_Event);
}

DXFence::~DXFence()
{
	Wait();
	CloseHandle(m_Event);
}

void DXFence::Signal(CommandType queue)
{
	++m_Value;
	DXDevice::Instance().SignalFence(m_Fence.get(), m_Value, queue);
}

void DXFence::Wait()
{
	if (!Ready())
	{
		m_Fence->SetEventOnCompletion(m_Value, m_Event);
		WaitForSingleObjectEx(m_Event, INFINITE, FALSE);
	}
}

bool DXFence::Ready() const
{
	return m_Fence->GetCompletedValue() == m_Value;
}
