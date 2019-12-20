#pragma once

#include "stdafx.h"

#include <d3d12.h>
#include <array>


struct VertexInputElement
{
	constexpr bool operator!=(const VertexInputElement& other) const
	{
		return other.m_Format != m_Format
			|| other.m_Offset != m_Offset
			|| std::strcmp(other.m_Semantic, m_Semantic) != 0
			|| other.m_SemanticIndex != m_SemanticIndex;
	}

	constexpr bool operator==(const VertexInputElement& other) const
	{
		return !(*this != other);
	}

	const char* m_Semantic;
	uint32_t	m_SemanticIndex;
	DXGI_FORMAT	m_Format;
	uint32_t	m_Offset;
};


class VertexInputElementArrayView
{
public:
	explicit constexpr VertexInputElementArrayView(const VertexInputElement* data)
		: m_Data(data)
	{
	}

	constexpr uint32_t Count() const
	{
		uint32_t count = 0u;
		const VertexInputElement* ptr = m_Data;
		while (ptr->m_Semantic)
		{
			++count;
			++ptr;
		}

		return count;
	}

	constexpr const VertexInputElement* begin() const
	{
		return m_Data;
	}

	constexpr const VertexInputElement* end() const
	{
		const VertexInputElement* endPtr = m_Data;
		while (endPtr->m_Semantic)
			++endPtr;

		return endPtr;
	}

private:
	const VertexInputElement* m_Data;
};


enum class VertexLayout : uint8_t
{
	PositionOnly = 0,
	PositionUV,
	PositionUVNormal,
	Count
};


class VertexLayoutMngr
{
private:
	static constexpr VertexInputElement	s_VertexLayouts[] =
	{
			{"POSITION",	0u,	DXGI_FORMAT_R32G32B32_FLOAT,	0u},
			{},

			{"POSITION",	0u,	DXGI_FORMAT_R32G32B32_FLOAT,	0u},
			{"TEXCOORD",	0u,	DXGI_FORMAT_R32G32_FLOAT,		sizeof(float) * 3u},
			{},

			{"POSITION",	0u,	DXGI_FORMAT_R32G32B32_FLOAT,	0u},
			{"NORMAL",		0u,	DXGI_FORMAT_R32G32B32_FLOAT,	sizeof(float) * 3u},
			{"TEXCOORD",	0u,	DXGI_FORMAT_R32G32_FLOAT,		sizeof(float) * 5u},
			{}
	};

	static constexpr const VertexInputElement* GetEntry(uint32_t num)
	{
		const VertexInputElement* ptr = s_VertexLayouts;
		while (num)
		{
			while (ptr->m_Semantic)
			{
				++ptr;
			}
			--num;
			++ptr;
		}

		return ptr;
	}

public:
	static constexpr const VertexInputElementArrayView Lookup(VertexLayout layout)
	{
		return VertexInputElementArrayView(GetEntry(static_cast<uint32_t>(layout)));
	}
};
