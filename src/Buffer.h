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


class ConstantBuffer : public Resource
{
public:
	ConstantBuffer(size_t size);
	ConstantBuffer(uint32_t size);
	ConstantBuffer(const ConstantBuffer& other) = delete;
	~ConstantBuffer();

	const DescriptorHandle& CBVHandle() const { return m_CBVHandle; }

	template<typename T>
	void SetData(uint32_t offset, const T& data)
	{
		std::memcpy(m_Data + offset, reinterpret_cast<const void*>(&data), sizeof(T));
	}

private:
	DescriptorHandleWithIdx		m_CBVHandle;
	uint8_t*					m_Data;
};
