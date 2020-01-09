#pragma once

#include "stdafx.h"
#include "DescriptorHeap.h"
#include "Resource.h"
#include "CommandType.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class Window;
class RenderTarget;

class DXDevice
{
public:
	~DXDevice() = default;
	DXDevice(const DXDevice&) = delete;

	static void InitFromWindow(const Window& window);
	static void Destroy();

	static DXDevice& Instance() { return *s_Instance; }

	ID3D12GraphicsCommandList* CreateCommandList(ID3D12CommandAllocator* allocator, D3D12_COMMAND_LIST_TYPE type);
	ID3D12CommandAllocator* CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type);
	ID3D12Fence* CreateFence();
	ID3D12DescriptorHeap* CreateDescriptorHeap(uint32_t* outHandleSize, D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32_t maxCount);
	DescriptorHandleWithIdx CreateRTVHandle(ID3D12Resource* renderTarget);
	DescriptorHandleWithIdx CreateDSVHandle(ID3D12Resource* depthBuffer, DXGI_FORMAT format=DXGI_FORMAT_FORCE_UINT);
	DescriptorHandleWithIdx CreateCBVHandle(ID3D12Resource* constantBuffer);
	DescriptorHandleWithIdx CreateSRVHandle(ID3D12Resource* resource, DXGI_FORMAT format=DXGI_FORMAT_FORCE_UINT);
	DescriptorHandleWithIdx CreateSamplerHandle(D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE UMode, D3D12_TEXTURE_ADDRESS_MODE VMode, D3D12_COMPARISON_FUNC cmpFunc);
	ID3D12RootSignature* CreateRootSignature(const std::vector<D3D12_ROOT_PARAMETER1>& params);
	ID3D12Resource* CreateCommitedResource(ResourceDimension dimension, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth, D3D12_RESOURCE_STATES initialState, ResourceFlags flags, const glm::vec4& clearColor = {}, DXGI_FORMAT clearFormat=DXGI_FORMAT_FORCE_UINT);
	ID3D12Resource* CreateCommitedUploadResource(const D3D12_RESOURCE_DESC& desc, const void* data, size_t dataSize);
	ID3D12PipelineState* CreatePSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);
	ID3D12PipelineState* CreatePSO(const D3D12_COMPUTE_PIPELINE_STATE_DESC& desc);

	void Submit(ID3D12CommandList* cmdList, CommandType destQueue);
	void SignalFence(ID3D12Fence* fence, uint64_t value, CommandType destQueue);
	void Present() { m_SwapChain->Present(0, 0); }
	void ReleaseRTVDescriptor(DescriptorHandleWithIdx& handle) { m_RTVHeap->FreeHandle(handle); }
	void ReleaseSRVDescriptor(DescriptorHandleWithIdx& handle) { m_SRVHeap->FreeHandle(handle); }
	void ReleaseDSVDescriptor(DescriptorHandleWithIdx& handle) { m_DSVHeap->FreeHandle(handle); }
	void SetDescriptorHeaps(ID3D12GraphicsCommandList* cmdList);

	std::vector<std::unique_ptr<RenderTarget>> CreateSwapChainTargets();

private:
	DXDevice(const Window& window);
	ID3D12CommandQueue* CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type);

	static std::unique_ptr<DXDevice>		s_Instance;

	ReleasedUniquePtr<ID3D12Device>			m_Device;
	ReleasedUniquePtr<IDXGISwapChain1>		m_SwapChain;
	ReleasedUniquePtr<ID3D12CommandQueue>	m_CmdQueue;
	ReleasedUniquePtr<ID3D12CommandQueue>	m_CopyQueue;

	std::unique_ptr<DescriptorHeap>			m_SRVHeap;
	std::unique_ptr<DescriptorHeap>			m_RTVHeap;
	std::unique_ptr<DescriptorHeap>			m_DSVHeap;
	std::unique_ptr<DescriptorHeap>			m_SamplerHeap;
};
