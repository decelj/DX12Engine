#include "stdafx.h"
#include "PSOBuilder.h"
#include "DXDevice.h"
#include "d3dx12.h"

#include <algorithm>
#include <iterator>

PSOBuilder::PSOBuilder()
	: m_RootSignature(nullptr)
	, m_VertexShader(nullptr)
	, m_PixelShader(nullptr)
	, m_ComputeShader(nullptr)
	, m_RTVFormats({})
	, m_DSVFormat(DXGI_FORMAT_UNKNOWN)
{
}

ID3D12PipelineState* PSOBuilder::Build()
{
	if (!m_ComputeShader)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = m_RootSignature;
		desc.VS.pShaderBytecode = m_VertexShader->GetBufferPointer();
		desc.VS.BytecodeLength = m_VertexShader->GetBufferSize();

		if (m_PixelShader)
		{
			desc.PS.pShaderBytecode = m_PixelShader->GetBufferPointer();
			desc.PS.BytecodeLength = m_PixelShader->GetBufferSize();
		}

		desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		desc.RasterizerState.FrontCounterClockwise = true;
		desc.InputLayout.NumElements = (uint32_t)m_InputElements.size();
		desc.InputLayout.pInputElementDescs = m_InputElements.data();
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets = (uint32_t)std::count_if(m_RTVFormats.begin(), m_RTVFormats.end(),
			[](DXGI_FORMAT& format)
			{
				return format != DXGI_FORMAT_UNKNOWN;
			}
		);
		std::copy(m_RTVFormats.begin(), m_RTVFormats.end(), desc.RTVFormats);
		desc.DSVFormat = m_DSVFormat;
		desc.SampleDesc.Count = 1u;
		desc.SampleMask = UINT32_MAX;

		if (m_DSVFormat != DXGI_FORMAT_UNKNOWN)
		{
			desc.DepthStencilState.DepthEnable = true;
			desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

		}

		return DXDevice::Instance().CreatePSO(desc);
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
