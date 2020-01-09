#include "stdafx.h"
#include "DXCommandList.h"
#include "DXDevice.h"
#include "DXFence.h"
#include "HRException.h"

DXCommandList::DXCommandList(CommandType type)
	: m_Type(type)
	, m_State(State::CLOSED)
	, m_AllocatorIdx(0u)
	, m_CmdList(nullptr)
	, m_CmdAllocators({})
	, m_Fences({})
{
	DXDevice& device = DXDevice::Instance();

	D3D12_COMMAND_LIST_TYPE nativeType = D3D12_COMMAND_LIST_TYPE_DIRECT;
	switch (type)
	{
	case CommandType::GRAPHICS:
		nativeType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		break;
	case CommandType::COMPUTE:
		nativeType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		break;
	case CommandType::COPY:
		nativeType = D3D12_COMMAND_LIST_TYPE_COPY;
		break;
	default:
		assert(false);
		break;
	}

	for (AllocatorPtr& allocator : m_CmdAllocators)
	{
		allocator.reset(device.CreateCommandAllocator(nativeType));
	}

	for (FencePtr& fence : m_Fences)
	{
		fence = std::make_unique<DXFence>();
	}

	m_CmdList.reset(device.CreateCommandList(m_CmdAllocators[m_AllocatorIdx].get(), nativeType));
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

	if (m_Type != CommandType::COPY)
	{
		DXDevice::Instance().SetDescriptorHeaps(m_CmdList.get());
	}
}

void DXCommandList::End()
{
	assert(m_State == State::OPEN);

	m_State = State::CLOSED;
	m_CmdList->Close();
}

void DXCommandList::Submit()
{
	assert(m_State == State::CLOSED);

	DXDevice& device = DXDevice::Instance();
	device.Submit(m_CmdList.get(), m_Type);
	m_Fences[m_AllocatorIdx]->Signal();
}
