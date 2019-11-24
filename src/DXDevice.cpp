#include "stdafx.h"
#include "DXDevice.h"
#include "Window.h"
#include "HRException.h"
#include "d3dx12.h"
#include "RootSignatureBuilder.h"
#include "RenderTarget.h"
#include "d3dx12.h"

#include <dxgidebug.h>

std::unique_ptr<DXDevice> DXDevice::s_Instance = nullptr;

namespace
{
void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter, D3D_FEATURE_LEVEL featureLevel)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), featureLevel, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}
} // anonymous namespace


DXDevice::DXDevice(const Window& window)
	: m_Device(nullptr)
	, m_SwapChain(nullptr)
	, m_CmdQueue(nullptr)
	, m_CopyQueue(nullptr)
	, m_SRVHeap(nullptr)
	, m_RTVHeap(nullptr)
{
	UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;

		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))

		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	constexpr D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	ComPtr<IDXGIAdapter1> hardwareAdapter;
	GetHardwareAdapter(factory.Get(), &hardwareAdapter, featureLevel);

	{
		ID3D12Device* device = nullptr;
		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			featureLevel,
			IID_PPV_ARGS(&device)
		));
		m_Device.reset(device);
	}

	m_CmdQueue.reset(CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
	m_CopyQueue.reset(CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	//swapChainDesc.Width = window.Width();
	//swapChainDesc.Height = window.Height();
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	{
		IDXGISwapChain1* swapChain = nullptr;
		ThrowIfFailed(factory->CreateSwapChainForHwnd(
			m_CmdQueue.get(),        // Swap chain needs the queue so that it can force a flush on it.
			window.Handle(),
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		));
		m_SwapChain.reset(swapChain);
	}

	ThrowIfFailed(factory->MakeWindowAssociation(window.Handle(), DXGI_MWA_NO_ALT_ENTER));

	constexpr uint32_t kMaxHandles = 32u; // TODO: Fix me!
	m_RTVHeap = std::make_unique<DescriptorHeap>(*this, DescriptorType::RTV, kMaxHandles);
	m_SRVHeap = std::make_unique<DescriptorHeap>(*this, DescriptorType::SRV, kMaxHandles);
}

void DXDevice::InitFromWindow(const Window& window)
{
	assert(s_Instance == nullptr);
	s_Instance.reset(new DXDevice(window));
}

void DXDevice::Destroy()
{
	s_Instance.reset();

#ifdef _DEBUG
	{
		ComPtr<IDXGIDebug1> dxgiDebug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
		{
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}
	}
#endif
}

ID3D12CommandQueue* DXDevice::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type)
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = type;

	ID3D12CommandQueue* cmdQueue = nullptr;
	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue)));

	return cmdQueue;
}

ID3D12GraphicsCommandList* DXDevice::CreateCommandList(ID3D12CommandAllocator* allocator, D3D12_COMMAND_LIST_TYPE type)
{
	ID3D12GraphicsCommandList* cmdList = nullptr;
	ThrowIfFailed(
		m_Device->CreateCommandList(0, type, allocator, nullptr, IID_PPV_ARGS(&cmdList)));

	return cmdList;
}

ID3D12CommandAllocator* DXDevice::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
{
	ID3D12CommandAllocator* allocator = nullptr;
	ThrowIfFailed(
		m_Device->CreateCommandAllocator(type, IID_PPV_ARGS(&allocator)));

	return allocator;
}

ID3D12Fence* DXDevice::CreateFence()
{
	ID3D12Fence* fence = nullptr;
	ThrowIfFailed(
		m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

	return fence;
}

ID3D12DescriptorHeap* DXDevice::CreateDescriptorHeap(uint32_t* outHandleSize, D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32_t maxCount)
{
	assert(outHandleSize != nullptr);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = maxCount;
	heapDesc.Type = heapType;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ID3D12DescriptorHeap* heap = nullptr;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)));

	*outHandleSize = m_Device->GetDescriptorHandleIncrementSize(heapType);
	return heap;
}

DescriptorHandleWithIdx DXDevice::CreateRTVHandle(ID3D12Resource* renderTarget)
{
	D3D12_RESOURCE_DESC resDesc = renderTarget->GetDesc();
	D3D12_RENDER_TARGET_VIEW_DESC desc = {};
	desc.Format = resDesc.Format;

#define CASE(_type) \
	case D3D12_RESOURCE_DIMENSION_##_type: \
	desc.ViewDimension = D3D12_RTV_DIMENSION_##_type; break

	switch (resDesc.Dimension)
	{
		CASE(BUFFER);
		CASE(TEXTURE1D);
		CASE(TEXTURE2D);
		CASE(TEXTURE3D);
	default:
		assert(false);
		break;
	}
#undef CASE

	DescriptorHandleWithIdx handle = m_RTVHeap->AllocateHandle();
	m_Device->CreateRenderTargetView(renderTarget, &desc, handle.cpuHandle);

	return handle;
}

ID3D12RootSignature* DXDevice::CreateRootSignature(const std::vector<D3D12_ROOT_PARAMETER1>& params)
{
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC desc = {};
	desc.Desc_1_1.NumParameters = (uint32_t)params.size();
	desc.Desc_1_1.pParameters = params.data();
	desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;

	ID3DBlob* serialized = nullptr;
	ID3DBlob* error = nullptr;
	ThrowIfFailed(D3D12SerializeVersionedRootSignature(&desc, &serialized, &error));

	ID3D12RootSignature* rootSig = nullptr;
	ThrowIfFailed(m_Device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&rootSig)));

	return rootSig;
}

ID3D12Resource* DXDevice::CreateCommitedResource(ResourceDimension dimension, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth, D3D12_RESOURCE_STATES initialState)
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Format = format;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.DepthOrArraySize = depth;
	desc.SampleDesc.Count = 1;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	switch (dimension)
	{
	case ResourceDimension::Linear:
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		break;
	case ResourceDimension::Texture2D:
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		break;
	case ResourceDimension::Texture3D:
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		break;
	default:
		assert(false);
		break;
	}

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	ID3D12Resource* resource = nullptr;
	ThrowIfFailed(
		m_Device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc, initialState, nullptr, IID_PPV_ARGS(&resource)));

	return resource;
}

ID3D12Resource* DXDevice::CreateCommitedUploadResource(const D3D12_RESOURCE_DESC& desc, void* data, size_t dataSize)
{
	assert(data);
	assert(dataSize);

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	ID3D12Resource* resource = nullptr;
	ThrowIfFailed(
		m_Device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource)));

	uint8_t* gpuData = nullptr;
	ThrowIfFailed(resource->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&gpuData)));
	memcpy(gpuData, data, dataSize);
	resource->Unmap(0, nullptr);

	return resource;
}

void DXDevice::Submit(ID3D12CommandList* cmdList, CommandType destQueue)
{
	ID3D12CommandList* cmdLists[] = { cmdList };

	switch (destQueue)
	{
	case CommandType::GRAPHICS:
		m_CmdQueue->ExecuteCommandLists(1u, cmdLists);
		break;
	case CommandType::COMPUTE:
		assert(false); // TODO
		break;
	case CommandType::COPY:
		m_CopyQueue->ExecuteCommandLists(1u, cmdLists);
		break;
	default:
		assert(false);
		break;
	}
}

void DXDevice::SignalFence(ID3D12Fence* fence, uint64_t value, CommandType destQueue)
{
	switch (destQueue)
	{
	case CommandType::GRAPHICS:
		m_CmdQueue->Signal(fence, value);
		break;
	case CommandType::COMPUTE:
		assert(false); // TODO
		break;
	case CommandType::COPY:
		m_CopyQueue->Signal(fence, value);
		break;
	default:
		assert(false);
		break;
	}
}

std::vector<std::unique_ptr<RenderTarget>> DXDevice::CreateSwapChainTargets()
{
	using RenderTargetPtr = std::unique_ptr<RenderTarget>;
	std::vector<RenderTargetPtr> outTargets(kNumFramesInFlight);

	for (uint32_t i = 0; i < kNumFramesInFlight; ++i)
	{
		ID3D12Resource* buffer = nullptr;
		m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&buffer));
		outTargets[i] = std::make_unique<RenderTarget>(buffer, ResourceState::Common);
	}

	return outTargets;
}
