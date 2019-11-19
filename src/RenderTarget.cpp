#include "stdafx.h"
#include "RenderTarget.h"
#include "DXDevice.h"
#include "DXCommandList.h"

RenderTarget::RenderTarget(DXDevice& device, DXGI_FORMAT format, uint32_t width, uint32_t height)
	: m_Texture(nullptr)
	, m_CurrentState(ResourceState::Common)
	, m_Format(format)
	, m_Width(width)
	, m_Height(height)
	, m_IsOwner(true)
{
	m_Texture.reset(device.CreateCommitedResource(format, width, height));
	m_RTVHandle = device.CreateRTVHandle(m_Texture.get());
}

RenderTarget::RenderTarget(DXDevice& device, ID3D12Resource* texture, ResourceState currentState)
	: m_Texture(texture)
	, m_CurrentState(currentState)
	, m_IsOwner(false)
{
	assert(m_Texture != nullptr);

	D3D12_RESOURCE_DESC resDesc = m_Texture->GetDesc();
	m_Format = resDesc.Format;
	m_Width = (uint32_t)resDesc.Width;
	m_Height = (uint32_t)resDesc.Height;

	m_RTVHandle = device.CreateRTVHandle(m_Texture.get());
}

RenderTarget::~RenderTarget()
{
	DXDevice::Instance().ReleaseRTVDescriptor(m_RTVHandle);
	if (!m_IsOwner)
	{
		m_Texture.release();
	}
}

void RenderTarget::TransitionTo(ResourceState destState, DXCommandList& cmdList)
{
	D3D12_RESOURCE_BARRIER barrier = CreateBarrier(m_Texture.get(), m_CurrentState, destState);
	m_CurrentState = destState;

	cmdList.Native()->ResourceBarrier(1, &barrier);
}
