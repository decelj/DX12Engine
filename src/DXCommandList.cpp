#include "stdafx.h"
#include "DXCommandList.h"
#include "DXDevice.h"
#include "DXFence.h"
#include "HRException.h"

DXCommandList::DXCommandList(DXDevice& device)
	: m_State(State::CLOSED)
	, m_AllocatorIdx(0u)
	, m_CmdList(nullptr)
	, m_CmdAllocators({})
	, m_Fences({})
{
	for (AllocatorPtr& allocator : m_CmdAllocators)
	{
		allocator.reset(device.CreateCommandAllocator());
	}

	for (FencePtr& fence : m_Fences)
	{
		fence = std::make_unique<DXFence>(device);
	}

	m_CmdList.reset(device.CreateCommandList(m_CmdAllocators[m_AllocatorIdx].get()));
	m_CmdList->Close();
}

DXCommandList::~DXCommandList()
{
	if (m_State != State::CLOSED)
	{
		End();
	}

	for (FencePtr& fence : m_Fences)
	{
		fence->Wait();
	}
}

void DXCommandList::Begin(ID3D12PipelineState* pso)
{
	assert(m_State == State::CLOSED);

	m_State = State::OPEN;
	m_AllocatorIdx = (m_AllocatorIdx + 1u) % kNumFramesInFlight;

	m_Fences[m_AllocatorIdx]->Wait();
	m_CmdAllocators[m_AllocatorIdx]->Reset();
	ThrowIfFailed(m_CmdList->Reset(m_CmdAllocators[m_AllocatorIdx].get(), pso));
}

void DXCommandList::End()
{
	assert(m_State == State::OPEN);

	m_State = State::CLOSED;
	m_CmdList->Close();
}

void DXCommandList::Submit(DXDevice& device)
{
	device.Submit(m_CmdList.get());
	m_Fences[m_AllocatorIdx]->Signal(device);
}
