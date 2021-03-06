#include "stdafx.h"
#include "DXDevice.h"
#include "Resource.h"
#include "DXCommandList.h"
#include "HRException.h"

Resource::Resource(ResourceDimension dimension, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth, ResourceState initialState, ResourceFlags flags, const glm::vec4& clearColor)
	: m_Resource(nullptr)
	, m_CurrentState(initialState)
{
	DXGI_FORMAT clearFormat = format;
	switch (format)
	{
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		format = DXGI_FORMAT_R24G8_TYPELESS;
		break;
	case DXGI_FORMAT_D32_FLOAT:
		format = DXGI_FORMAT_R32_TYPELESS;
		break;
	default:
		break;
	}

	DXDevice& device = DXDevice::Instance();
	m_Resource.reset(
		device.CreateCommitedResource(
			dimension, format, width, height, depth,
			static_cast<D3D12_RESOURCE_STATES>(initialState), flags,
			clearColor, clearFormat
		));
}

Resource::Resource(ResourceState initialState)
	: m_Resource(nullptr)
	, m_CurrentState(initialState)
{
}

uint8_t* Resource::Map()
{
	uint8_t* data = nullptr;
	D3D12_RANGE range = {};
	ThrowIfFailed(m_Resource->Map(0, &range, reinterpret_cast<void**>(&data)));
	return data;
}

void Resource::TransitionTo(ResourceState destState, DXCommandList& cmdList)
{
	if (destState == m_CurrentState)
	{
		return;
	}

	D3D12_RESOURCE_BARRIER barrier = CreateBarrier(m_Resource.get(), m_CurrentState, destState);
	m_CurrentState = destState;

	cmdList.Native()->ResourceBarrier(1, &barrier);
}
