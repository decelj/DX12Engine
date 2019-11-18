#include "stdafx.h"
#include "DescriptorHeap.h"
#include "DXDevice.h"
#include "d3dx12.h"

#include <numeric>

DescriptorHeap::DescriptorHeap(DXDevice& device, DescriptorType type, uint32_t maxCount)
{
	m_FreeHandles.resize(maxCount);
	std::iota(m_FreeHandles.begin(), m_FreeHandles.end(), 0);

	D3D12_DESCRIPTOR_HEAP_TYPE dxType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	switch (type)
	{
	case DescriptorType::RTV:
		dxType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		break;
	case DescriptorType::SRV:
		dxType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		break;
	default:
		assert(0);
		break;
	}

	m_Heap.reset(device.CreateDescriptorHeap(&m_HandleSize, dxType, maxCount));
}

DescriptorHeap::~DescriptorHeap()
{
}

DescriptorHandleWithIdx DescriptorHeap::AllocateHandle()
{
	assert(!m_FreeHandles.empty());

	DescriptorHandleWithIdx outHandle = {};
	outHandle.idx = m_FreeHandles.back();
	m_FreeHandles.pop_back();

	outHandle.cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_Heap->GetCPUDescriptorHandleForHeapStart(), outHandle.idx, m_HandleSize);
	outHandle.gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_Heap->GetGPUDescriptorHandleForHeapStart(), outHandle.idx, m_HandleSize);

	return outHandle;
}

void DescriptorHeap::FreeHandle(const DescriptorHandleWithIdx& desc)
{
#ifdef _DEBUG
	assert(std::find(m_FreeHandles.begin(), m_FreeHandles.end(), desc.idx) == m_FreeHandles.end());
#endif // _DEBUG

	m_FreeHandles.emplace_back(desc.idx);
}
