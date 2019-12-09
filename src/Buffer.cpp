#include "stdafx.h"
#include "Buffer.h"
#include "DXDevice.h"
#include "Utils.h"

VertexBuffer::VertexBuffer(uint32_t size, uint32_t stride)
	: Resource(ResourceDimension::Linear, DXGI_FORMAT_UNKNOWN, size, 1, 1, ResourceState::Common, ResourceFlags::NonShaderResource)
{
	m_View.SizeInBytes = size;
	m_View.StrideInBytes = stride;
	m_View.BufferLocation = m_Resource->GetGPUVirtualAddress();
}

IndexBuffer::IndexBuffer(uint32_t count)
	: Resource(ResourceDimension::Linear, DXGI_FORMAT_UNKNOWN, 2u * count, 1, 1, ResourceState::Common, ResourceFlags::NonShaderResource)
{
	assert(count < 0xFFFFu);

	m_View.Format = DXGI_FORMAT_R16_UINT;
	m_View.SizeInBytes = 2u * count;
	m_View.BufferLocation = m_Resource->GetGPUVirtualAddress();
}

ConstantBuffer::ConstantBuffer(uint32_t size)
	: Resource(ResourceDimension::Linear, DXGI_FORMAT_UNKNOWN, AlignTo<256u>(size), 1, 1, ResourceState::GenericRead, ResourceFlags::CPUMappable)
	, m_Data(nullptr)
{
	m_CBVHandle = DXDevice::Instance().CreateCBVHandle(m_Resource.get());
	m_Data = Map();
}

ConstantBuffer::~ConstantBuffer()
{
	Unmap();
	DXDevice::Instance().ReleaseSRVDescriptor(m_CBVHandle);
}
