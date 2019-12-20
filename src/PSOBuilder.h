#pragma once

#include "stdafx.h"
#include "VertexLayoutMngr.h"

#include <d3d12.h>
#include <algorithm>


class PSOBuilder
{
public:
	PSOBuilder();
	~PSOBuilder() = default;

	void SetRootSignature(ID3D12RootSignature* rootSig)		{ m_RootSignature = rootSig; }
	void SetVertexShader(ID3DBlob* shader)					{ m_VertexShader = shader; }
	void SetPixelShader(ID3DBlob* shader)					{ m_PixelShader = shader; }
	void SetComputeShader(ID3DBlob* shader)					{ m_ComputeShader = shader; }
	void SetDSVFormat(DXGI_FORMAT dsvFormat)				{ m_DSVFormat = dsvFormat; }

	void SetVertexLayout(VertexLayout layout)
	{
		const VertexInputElementArrayView& elements = VertexLayoutMngr::Lookup(layout);

		m_InputElements.clear();
		m_InputElements.reserve(elements.Count());
		std::transform(elements.begin(), elements.end(), std::back_inserter(m_InputElements),
			[](const VertexInputElement& elem)
			{
				D3D12_INPUT_ELEMENT_DESC desc = {};
				desc.Format = elem.m_Format;
				desc.AlignedByteOffset = elem.m_Offset;
				desc.InputSlot = 0u;
				desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				desc.SemanticName = elem.m_Semantic;
				desc.SemanticIndex = elem.m_SemanticIndex;
				desc.InstanceDataStepRate = 0u;
				return desc;
			});
	}

	template<size_t ASize>
	void SetRTVFormats(const std::array<DXGI_FORMAT, ASize>& formats)
	{
		std::copy(formats.begin(), formats.end(), m_RTVFormats.begin());
	}

	ID3D12PipelineState* Build();

private:
	ID3D12RootSignature*	m_RootSignature;
	ID3DBlob*				m_VertexShader;
	ID3DBlob*				m_PixelShader;
	ID3DBlob*				m_ComputeShader;

	std::vector<D3D12_INPUT_ELEMENT_DESC>	m_InputElements;
	std::array<DXGI_FORMAT, 8u>				m_RTVFormats;
	DXGI_FORMAT								m_DSVFormat;
};
