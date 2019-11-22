#include "stdafx.h"
#include "RenderTarget.h"
#include "DXDevice.h"
#include "DXCommandList.h"

RenderTarget::RenderTarget(ResourceDimension dimension, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth)
	: Resource(dimension, format, width, height, depth, nullptr, 0, ResourceState::Common)
	, m_IsOwner(true)
{
	m_RTVHandle = DXDevice::Instance().CreateRTVHandle(m_Resource.get());
}

RenderTarget::RenderTarget(ID3D12Resource* texture, ResourceState currentState)
	: Resource(currentState)
	, m_IsOwner(false)
{
	assert(texture != nullptr);

	m_Resource.reset(texture);
	m_RTVHandle = DXDevice::Instance().CreateRTVHandle(texture);
}

RenderTarget::~RenderTarget()
{
	DXDevice::Instance().ReleaseRTVDescriptor(m_RTVHandle);
	if (!m_IsOwner)
	{
		m_Resource.release();
	}
}
