#include "stdafx.h"
#include "Buffer.h"

VertexBuffer::VertexBuffer(DXGI_FORMAT format, uint32_t size, uint32_t stride)
	: Resource(ResourceDimension::Linear, format, size, 1, 1, ResourceState::Common)
{
	m_View.SizeInBytes = size;
	m_View.StrideInBytes = stride;
	m_View.BufferLocation = m_Resource->GetGPUVirtualAddress();
}
