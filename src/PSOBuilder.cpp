#include "stdafx.h"
#include "PSOBuilder.h"
#include "DXDevice.h"
#include "d3dx12.h"

#include <algorithm>
#include <iterator>

PSOBuilder::PSOBuilder()
	: m_VertexShader(nullptr)
	, m_PixelShader(nullptr)
	, m_ComputeShader(nullptr)
{
}

ID3D12PipelineState* PSOBuilder::Build(DXDevice& device)
{
	if (!m_ComputeShader)
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputDescs;
		inputDescs.reserve(m_InputElements.size());
		std::transform(m_InputElements.begin(), m_InputElements.end(), std::back_inserter(inputDescs),
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

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = m_RootSignature;
		desc.VS.pShaderBytecode = m_VertexShader->GetBufferPointer();
		desc.VS.BytecodeLength = m_VertexShader->GetBufferSize();
		desc.PS.pShaderBytecode = m_PixelShader->GetBufferPointer();
		desc.PS.BytecodeLength = m_PixelShader->GetBufferSize();
		desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		desc.InputLayout.NumElements = (uint32_t)m_InputElements.size();
		desc.InputLayout.pInputElementDescs = inputDescs.data();
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets = (uint32_t)std::count_if(m_RTVFormats.begin(), m_RTVFormats.end(),
			[](DXGI_FORMAT& format)
			{
				return format != DXGI_FORMAT_UNKNOWN;
			}
		);
		std::copy(m_RTVFormats.begin(), m_RTVFormats.end(), desc.RTVFormats);
		desc.SampleDesc.Count = 1u;
		desc.SampleMask = UINT32_MAX;

		return device.CreatePSO(desc);
	}
	else
	{
		// TODO
		assert(false);
		assert(m_VertexShader == nullptr);
		assert(m_PixelShader == nullptr);
	}

	return nullptr;
}
