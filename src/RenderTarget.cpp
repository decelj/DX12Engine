#include "stdafx.h"
#include "RenderTarget.h"
#include "DXDevice.h"
#include "DXCommandList.h"

RenderTarget::RenderTarget(ResourceDimension dimension, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth, const glm::vec4& clearColor)
	: Resource(dimension, format, width, height, depth, ResourceState::Common, ResourceFlags::RenderTarget, clearColor)
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

DepthTarget::DepthTarget(DXGI_FORMAT format, uint32_t width, uint32_t height, float clearValue)
	: Resource(ResourceDimension::Texture2D, format, width, height, 1, ResourceState::DepthWrite, ResourceFlags::DepthStencil, glm::vec4(clearValue))
{
	m_DSVHandle = DXDevice::Instance().CreateDSVHandle(m_Resource.get(), format);

	DXGI_FORMAT srvFormat = format;
	switch (format)
	{
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		break;
	case DXGI_FORMAT_D32_FLOAT:
		srvFormat = DXGI_FORMAT_R32_FLOAT;
		break;
	default:
		break;
	}
	m_SRVHandle = DXDevice::Instance().CreateSRVHandle(m_Resource.get(), srvFormat);
}

DepthTarget::~DepthTarget()
{
	DXDevice::Instance().ReleaseDSVDescriptor(m_DSVHandle);
}
