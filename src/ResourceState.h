#pragma once

#include "stdafx.h"
#include "d3dx12.h"

#include <d3d12.h>


enum class ResourceState
{
	Common = D3D12_RESOURCE_STATE_COMMON,
	VertexAndConstantBuffer = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
	IndexBuffer = D3D12_RESOURCE_STATE_INDEX_BUFFER,
	RenderTarget = D3D12_RESOURCE_STATE_RENDER_TARGET,
	UAV = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
	DepthWrite = D3D12_RESOURCE_STATE_DEPTH_WRITE,
	DepthRead = D3D12_RESOURCE_STATE_DEPTH_READ,
	NonPixelShaderResource = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
	PixelShaderResource = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	StreamOut = D3D12_RESOURCE_STATE_STREAM_OUT,
	IndirectArgument = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
	CopyDest = D3D12_RESOURCE_STATE_COPY_DEST,
	CopySource = D3D12_RESOURCE_STATE_COPY_SOURCE,
	ResolveDest = D3D12_RESOURCE_STATE_RESOLVE_DEST,
	ResolveSource = D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
	GenericRead = D3D12_RESOURCE_STATE_GENERIC_READ,
	RaytracingAccelStructure = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
	Present = D3D12_RESOURCE_STATE_PRESENT,
	ShadingRateSource = D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE,
	Prediction = D3D12_RESOURCE_STATE_PREDICATION
};

inline ResourceState operator&(const ResourceState lhs, const ResourceState rhs)
{
	using T = std::underlying_type_t<ResourceState>;
	return static_cast<ResourceState>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

inline ResourceState operator|(const ResourceState lhs, const ResourceState rhs)
{
	using T = std::underlying_type_t<ResourceState>;
	return static_cast<ResourceState>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline ResourceState& operator|=(ResourceState& lhs, const ResourceState rhs)
{
	lhs = lhs | rhs;
	return lhs;
}

inline D3D12_RESOURCE_BARRIER CreateBarrier(ID3D12Resource* resource, ResourceState current, ResourceState dest)
{
	return CD3DX12_RESOURCE_BARRIER::Transition(resource, static_cast<D3D12_RESOURCE_STATES>(current), static_cast<D3D12_RESOURCE_STATES>(dest));
}
