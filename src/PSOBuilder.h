#pragma once

#include "stdafx.h"

#include <d3d12.h>

class DXDevice;

struct VertexInputElement
{
	const char* m_Semantic;
	uint32_t	m_SemanticIndex;
	DXGI_FORMAT	m_Format;
	uint32_t	m_Offset;
};

class PSOBuilder
{
public:
	PSOBuilder();
	~PSOBuilder() = default;

	void SetRootSignature(ID3D12RootSignature* rootSig)		{ m_RootSignature = rootSig; }
	void SetVertexShader(ID3DBlob* shader)					{ m_VertexShader = shader; }
	void SetPixelShader(ID3DBlob* shader)					{ m_PixelShader = shader; }
	void SetComputeShader(ID3DBlob* shader)					{ m_ComputeShader = shader; }

	void AppendInputElements(const std::vector<VertexInputElement>& elems)
	{
		for (const VertexInputElement& elem : elems)
		{
			m_InputElements.emplace_back(elem);
		}
	}

	template<size_t ASize>
	void SetRTVFormats(const std::array<DXGI_FORMAT, ASize>& formats)
	{
		for (size_t i = 0u; i < ASize; ++i)
		{
			m_RTVFormats[i] = formats[i];
		}
	}

	ID3D12PipelineState* Build(DXDevice& device);

private:
	ID3D12RootSignature*	m_RootSignature;
	ID3DBlob*				m_VertexShader;
	ID3DBlob*				m_PixelShader;
	ID3DBlob*				m_ComputeShader;

	std::vector<VertexInputElement>	m_InputElements;
	std::array<DXGI_FORMAT, 8u>		m_RTVFormats;
};
