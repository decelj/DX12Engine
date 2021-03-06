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
	m_DSVHeap = std::make_unique<DescriptorHeap>(*this, DescriptorType::DSV, kMaxHandles);
	m_SamplerHeap = std::make_unique<DescriptorHeap>(*this, DescriptorType::Sampler, kMaxHandles);
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

	if (	heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		||	heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
	{
		heapDesc.Flags |= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	}

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

DescriptorHandleWithIdx DXDevice::CreateDSVHandle(ID3D12Resource* depthBuffer, DXGI_FORMAT format)
{
	D3D12_RESOURCE_DESC resDesc = depthBuffer->GetDesc();
	if (format == DXGI_FORMAT_FORCE_UINT)
	{
		format = resDesc.Format;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
	desc.Format = format;

#define CASE(_type) \
	case D3D12_RESOURCE_DIMENSION_##_type: \
	desc.ViewDimension = D3D12_DSV_DIMENSION_##_type; break

	switch (resDesc.Dimension)
	{
		CASE(TEXTURE1D);
		CASE(TEXTURE2D);
	default:
		assert(false);
		break;
	}
#undef CASE

	DescriptorHandleWithIdx handle = m_DSVHeap->AllocateHandle();
	m_Device->CreateDepthStencilView(depthBuffer, &desc, handle.cpuHandle);

	return handle;
}

DescriptorHandleWithIdx DXDevice::CreateCBVHandle(ID3D12Resource* constantBuffer)
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
	desc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
	desc.SizeInBytes = (uint32_t)constantBuffer->GetDesc().Width;

	DescriptorHandleWithIdx handle = m_SRVHeap->AllocateHandle();
	m_Device->CreateConstantBufferView(&desc, handle.cpuHandle);

	return handle;
}

DescriptorHandleWithIdx DXDevice::CreateSRVHandle(ID3D12Resource* resource, DXGI_FORMAT format)
{
	D3D12_RESOURCE_DESC resDesc = resource->GetDesc();
	if (format == DXGI_FORMAT_FORCE_UINT)
	{
		format = resDesc.Format;
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = format;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (resDesc.Dimension)
	{
	case D3D12_RESOURCE_DIMENSION_BUFFER:
		desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0u;
		assert(false); // TODO: Support buffers
		break;

	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
		desc.Texture1D.MipLevels = resDesc.MipLevels;
		break;

	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = resDesc.MipLevels;
		break;

	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MipLevels = resDesc.MipLevels;
		break;

	default:
		assert(false);
		break;
	}

	DescriptorHandleWithIdx handle = m_SRVHeap->AllocateHandle();
	m_Device->CreateShaderResourceView(resource, &desc, handle.cpuHandle);

	return handle;
}

DescriptorHandleWithIdx DXDevice::CreateSamplerHandle(D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE UMode, D3D12_TEXTURE_ADDRESS_MODE VMode, D3D12_COMPARISON_FUNC cmpFunc)
{
	// TODO: Finish me
	assert(0);

	D3D12_SAMPLER_DESC desc = {};
	desc.Filter = filter;
	desc.AddressU = UMode;
	desc.AddressV = VMode;
	desc.ComparisonFunc = cmpFunc;

	return DescriptorHandleWithIdx();
}

ID3D12RootSignature* DXDevice::CreateRootSignature(const std::vector<D3D12_ROOT_PARAMETER1>& params, const std::vector<D3D12_STATIC_SAMPLER_DESC>& staticSamplers)
{
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC desc = {};
	desc.Desc_1_1.NumParameters = (uint32_t)params.size();
	desc.Desc_1_1.pParameters = params.data();
	desc.Desc_1_1.NumStaticSamplers = (uint32_t)staticSamplers.size();
	desc.Desc_1_1.pStaticSamplers = staticSamplers.data();
	desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	desc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* serialized = nullptr;
	ID3DBlob* error = nullptr;
	ThrowIfFailed(D3D12SerializeVersionedRootSignature(&desc, &serialized, &error));

	ID3D12RootSignature* rootSig = nullptr;
	ThrowIfFailed(m_Device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&rootSig)));

	return rootSig;
}

ID3D12Resource* DXDevice::CreateCommitedResource(ResourceDimension dimension, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth, D3D12_RESOURCE_STATES initialState, ResourceFlags flags, const glm::vec4& clearColor, DXGI_FORMAT clearFormat)
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Format = format;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.DepthOrArraySize = depth;
	desc.SampleDesc.Count = 1;
	desc.Flags = static_cast<D3D12_RESOURCE_FLAGS>(static_cast<uint32_t>(flags) & 0xFFFF);

	D3D12_CLEAR_VALUE* clearValuePtr = nullptr;
	D3D12_CLEAR_VALUE clearValue = {};
	if (clearFormat == DXGI_FORMAT_FORCE_UINT)
	{
		clearFormat = format;
	}

	switch (dimension)
	{
	case ResourceDimension::Linear:
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		break;
	case ResourceDimension::Texture2D:
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		if (	(flags & ResourceFlags::RenderTarget) != ResourceFlags::None
			||	(flags & ResourceFlags::DepthStencil) != ResourceFlags::None)
		{
			clearValuePtr = &clearValue;
			clearValue.Format = clearFormat;
			if (clearFormat == DXGI_FORMAT_D24_UNORM_S8_UINT
				|| clearFormat == DXGI_FORMAT_D16_UNORM
				|| clearFormat == DXGI_FORMAT_D32_FLOAT)
			{
				clearValue.DepthStencil.Depth = clearColor.r;
			}
			else
			{
				std::copy((float*)&clearColor[0], (float*)&clearColor[3], clearValue.Color);
			}
		}

		break;
	case ResourceDimension::Texture3D:
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		break;
	default:
		assert(false);
		break;
	}

	D3D12_HEAP_PROPERTIES heapProps = {};
	if (static_cast<uint32_t>(flags) & static_cast<uint32_t>(ResourceFlags::CPUMappable))
	{
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	}
	else
	{
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	}

	ID3D12Resource* resource = nullptr;
	ThrowIfFailed(
		m_Device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc, initialState, clearValuePtr, IID_PPV_ARGS(&resource)));

	return resource;
}

ID3D12Resource* DXDevice::CreateCommitedUploadResource(const D3D12_RESOURCE_DESC& desc)
{
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	ID3D12Resource* resource = nullptr;
	ThrowIfFailed(
		m_Device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource)));

	return resource;
}

ID3D12PipelineState* DXDevice::CreatePSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
{
	ID3D12PipelineState* pso = nullptr;
	ThrowIfFailed(m_Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso)));
	return pso;
}

ID3D12PipelineState* DXDevice::CreatePSO(const D3D12_COMPUTE_PIPELINE_STATE_DESC& desc)
{
	ID3D12PipelineState* pso = nullptr;
	ThrowIfFailed(m_Device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pso)));
	return pso;
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

void DXDevice::SetDescriptorHeaps(ID3D12GraphicsCommandList* cmdList)
{
	assert(cmdList);

	ID3D12DescriptorHeap* heaps[] =
	{
		&m_SamplerHeap->Native(),
		&m_SRVHeap->Native()
	};
	cmdList->SetDescriptorHeaps(_countof(heaps), heaps);
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
