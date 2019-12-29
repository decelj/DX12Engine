#pragma once

#include "stdafx.h"
#include "CommandType.h"

#include <d3d12.h>

class DXDevice;
class DXFence;

class DXCommandList
{
public:
	DXCommandList(CommandType type);
	~DXCommandList();

	void Begin(ID3D12PipelineState* pso = nullptr);
	void End();
	void Submit();

	bool IsOpen() const { return m_State == State::OPEN; }

	ID3D12GraphicsCommandList* Native() { return m_CmdList.get(); }

private:
	enum class State : uint8_t
	{
		OPEN,
		CLOSED
	};

	using FencePtr = std::unique_ptr<DXFence>;
	using AllocatorPtr = ReleasedUniquePtr<ID3D12CommandAllocator>;
	using CommandListPtr = ReleasedUniquePtr<ID3D12GraphicsCommandList>;

	const CommandType			m_Type;
	State						m_State;
	uint8_t						m_AllocatorIdx;

	CommandListPtr				m_CmdList;
	IFFArray<AllocatorPtr>		m_CmdAllocators;
	IFFArray<FencePtr>			m_Fences;
};
