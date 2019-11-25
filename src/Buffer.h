#pragma once

#include "stdafx.h"
#include "DescriptorHeap.h"
#include "Resource.h"

#include <d3d12.h>

class VertexBuffer : public Resource
{
public:
	VertexBuffer(DXGI_FORMAT format, uint32_t size, uint32_t stride);
	VertexBuffer(const VertexBuffer& other) = delete;
	~VertexBuffer() = default;

	const D3D12_VERTEX_BUFFER_VIEW& View() const { return m_View; }

private:
	D3D12_VERTEX_BUFFER_VIEW	m_View;
};
