#pragma once

#include "stdafx.h"

#include <bitset>
#include <algorithm>
#include <d3d12.h>

class DXDevice;

class RootSignatureBuilder
{
public:
	RootSignatureBuilder();
	~RootSignatureBuilder();

	enum class RangeType
	{
		CBV = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		SRV = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		UAV = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		Sampler = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER
	};

	struct SimpleRange
	{
		RangeType m_Type;
		uint32_t m_StartReg;
		uint32_t m_EndReg;
		uint32_t m_Space;
	};

	template<uint32_t NumRanges>
	void SetTable(uint32_t slot, const std::array<SimpleRange, NumRanges>& table)
	{
		D3D12_ROOT_PARAMETER1 param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.DescriptorTable.NumDescriptorRanges = NumRanges;

		D3D12_DESCRIPTOR_RANGE1* ranges = new D3D12_DESCRIPTOR_RANGE1[NumRanges];
		uint32_t tableOffset = 0u;
		std::transform(table.begin(), table.end(), ranges,
			[&](const SimpleRange& inRange)
			{
				D3D12_DESCRIPTOR_RANGE1 range = {};
				range.RangeType = static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(inRange.m_Type);
				range.BaseShaderRegister = inRange.m_StartReg;
				range.NumDescriptors = (inRange.m_EndReg - inRange.m_StartReg) + 1u;
				range.OffsetInDescriptorsFromTableStart = tableOffset;
				range.RegisterSpace = inRange.m_Space;
				range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;

				tableOffset += range.NumDescriptors;

				return range;
			});

		param.DescriptorTable.pDescriptorRanges = ranges;

		if (m_Parameters.size() <= slot)
		{
			m_Parameters.resize(slot + 1ull);
		}

		m_Parameters[slot] = param;
		m_ParameterValid[slot] = true;
	}

	void SetSRV(uint32_t slot, uint32_t reg, uint32_t space) { SetDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, slot, reg, space); }
	void SetUAV(uint32_t slot, uint32_t reg, uint32_t space) { SetDescriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, slot, reg, space); }
	void SetCBV(uint32_t slot, uint32_t reg, uint32_t space) { SetDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, slot, reg, space); }
	void SetConstant(uint32_t slot, uint32_t reg, uint32_t space, uint32_t count);
	void SetStaticSampler(uint32_t slot, uint32_t reg, uint32_t space, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE UMode, D3D12_TEXTURE_ADDRESS_MODE VMode, D3D12_COMPARISON_FUNC cmpFunc);

	ID3D12RootSignature* Build();

private:
	void SetDescriptor(D3D12_ROOT_PARAMETER_TYPE type, uint32_t slot, uint32_t reg, uint32_t space);

	std::vector<D3D12_ROOT_PARAMETER1>		m_Parameters;
	std::bitset<32u>						m_ParameterValid;
	std::vector<D3D12_STATIC_SAMPLER_DESC>	m_StaticSamplers;
	std::bitset<32u>						m_SamplerValid;
};
