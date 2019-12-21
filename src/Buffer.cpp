#include "stdafx.h"
#include "Buffer.h"
#include "DXDevice.h"
#include "Utils.h"

VertexBuffer::VertexBuffer(uint32_t count, uint32_t vertexSize)
	: Resource(ResourceDimension::Linear, DXGI_FORMAT_UNKNOWN, count * vertexSize, 1, 1, ResourceState::Common, ResourceFlags::NonShaderResource)
{
	m_View.SizeInBytes = count * vertexSize;
	m_View.StrideInBytes = vertexSize;
	m_View.BufferLocation = m_Resource->GetGPUVirtualAddress();
}

IndexBuffer::IndexBuffer(uint32_t count)
	: Resource(ResourceDimension::Linear, DXGI_FORMAT_UNKNOWN, 4u * count, 1, 1, ResourceState::Common, ResourceFlags::NonShaderResource)
{
	m_View.Format = DXGI_FORMAT_R32_UINT;
	m_View.SizeInBytes = 4u * count;
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
