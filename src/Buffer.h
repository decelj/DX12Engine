#pragma once

#include "stdafx.h"
#include "DescriptorHeap.h"
#include "Resource.h"

#include <d3d12.h>

class VertexBuffer : public Resource
{
public:
	VertexBuffer(uint32_t count, uint32_t vertexSize);
	VertexBuffer(const VertexBuffer& other) = delete;
	~VertexBuffer() = default;

	const D3D12_VERTEX_BUFFER_VIEW& View() const { return m_View; }
	uint32_t Count() const { return m_View.SizeInBytes / m_View.StrideInBytes; }

private:
	D3D12_VERTEX_BUFFER_VIEW	m_View;
};


// Assumes uint32 indicies
class IndexBuffer : public Resource
{
public:
	IndexBuffer(uint32_t count);
	IndexBuffer(const IndexBuffer& other) = delete;
	~IndexBuffer() = default;

	const D3D12_INDEX_BUFFER_VIEW& View() const { return m_View; }
	uint32_t Count() const { return m_View.SizeInBytes >> 2u; }

private:
	D3D12_INDEX_BUFFER_VIEW		m_View;
};


class ConstantBuffer : public Resource
{
public:
	ConstantBuffer(uint32_t size);
	ConstantBuffer(const ConstantBuffer& other) = delete;
	~ConstantBuffer();

	const DescriptorHandle& CBVHandle() const { return m_CBVHandle; }

	template<typename T>
	void SetData(uint32_t offset, const T& data)
	{
		static_assert(!std::is_pointer<T>::value, "Must pass reference not pointer");
		std::memcpy(m_Data + offset, reinterpret_cast<const void*>(&data), sizeof(T));
	}

private:
	DescriptorHandleWithIdx		m_CBVHandle;
	uint8_t*					m_Data;
};
