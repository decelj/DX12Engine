#pragma once

#include "stdafx.h"
#include "DescriptorHeap.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class Window;
class RenderTarget;

class DXDevice
{
public:
	~DXDevice();
	DXDevice(const DXDevice&) = delete;

	static void InitFromWindow(const Window& window);
	static void Destroy();

	static DXDevice& Instance() { return *s_Instance; }

	ID3D12CommandQueue* CreateCommandQueue();
	ID3D12GraphicsCommandList* CreateCommandList(ID3D12CommandAllocator* allocator);
	ID3D12CommandAllocator* CreateCommandAllocator();
	ID3D12Fence* CreateFence();
	ID3D12DescriptorHeap* CreateDescriptorHeap(uint32_t* outHandleSize, D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32_t maxCount);
	DescriptorHandleWithIdx CreateRTVHandle(ID3D12Resource* renderTarget);
	ID3D12RootSignature* CreateRootSignature(const std::vector<D3D12_ROOT_PARAMETER1>& params);

	void Submit(ID3D12CommandList* cmdList);
	void SignalFence(ID3D12Fence* fence, uint64_t value);
	void Present() { m_SwapChain->Present(0, 0); }

	std::vector<std::unique_ptr<RenderTarget>> CreateSwapChainTargets();

private:
	DXDevice(const Window& window);

	static std::unique_ptr<DXDevice>		s_Instance;

	ReleasedUniquePtr<ID3D12Device>			m_Device;
	ReleasedUniquePtr<IDXGISwapChain1>		m_SwapChain;
	ReleasedUniquePtr<ID3D12CommandQueue>	m_CmdQueue;

	std::unique_ptr<DescriptorHeap>			m_SRVHeap;
	std::unique_ptr<DescriptorHeap>			m_RTVHeap;
};
