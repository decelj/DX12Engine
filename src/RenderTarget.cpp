#include "stdafx.h"
#include "RenderTarget.h"
#include "DXDevice.h"
#include "DXCommandList.h"

RenderTarget::RenderTarget(DXDevice& device, DXGI_FORMAT format, uint32_t width, uint32_t height)
{
	assert(false); // TODO!
}

RenderTarget::RenderTarget(DXDevice& device, ID3D12Resource* texture, ResourceState currentState)
	: m_Texture(texture)
	, m_CurrentState(currentState)
{
	assert(m_Texture != nullptr);

	D3D12_RESOURCE_DESC resDesc = m_Texture->GetDesc();
	m_Format = resDesc.Format;
	m_Width = (uint32_t)resDesc.Width;
	m_Height = (uint32_t)resDesc.Height;

	m_RTVHandle = device.CreateRTVHandle(m_Texture);
}

void RenderTarget::TransitionTo(ResourceState destState, DXCommandList& cmdList)
{
	D3D12_RESOURCE_BARRIER barrier = CreateBarrier(m_Texture, m_CurrentState, destState);
	m_CurrentState = destState;

	cmdList.Native()->ResourceBarrier(1, &barrier);
}
