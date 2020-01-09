#include "stdafx.h"
#include "RootSignatureBuilder.h"
#include "DXDevice.h"


RootSignatureBuilder::RootSignatureBuilder()
{
}

RootSignatureBuilder::~RootSignatureBuilder()
{
	for (D3D12_ROOT_PARAMETER1& param : m_Parameters)
	{
		if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			assert(param.DescriptorTable.pDescriptorRanges);
			delete[] param.DescriptorTable.pDescriptorRanges;
		}
	}
}

void RootSignatureBuilder::SetConstant(uint32_t slot, uint32_t reg, uint32_t space, uint32_t count)
{
	D3D12_ROOT_PARAMETER1 param = {};
	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	param.Constants.RegisterSpace = space;
	param.Constants.ShaderRegister = reg;
	param.Constants.Num32BitValues = count;

	if (m_Parameters.size() <= slot)
	{
		m_Parameters.resize(slot + 1ull);
	}

	m_Parameters[slot] = param;
	m_ParameterValid[slot] = true;
}

void RootSignatureBuilder::SetStaticSampler(uint32_t slot, uint32_t reg, uint32_t space, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE UMode, D3D12_TEXTURE_ADDRESS_MODE VMode, D3D12_COMPARISON_FUNC cmpFunc)
{
	D3D12_STATIC_SAMPLER_DESC desc = CD3DX12_STATIC_SAMPLER_DESC(reg);
	desc.Filter = filter;
	desc.AddressU = UMode;
	desc.AddressV = VMode;
	desc.ComparisonFunc = cmpFunc;
	desc.RegisterSpace = space;

	if (m_StaticSamplers.size() <= slot)
	{
		m_StaticSamplers.resize(slot + 1ull);
	}

	m_StaticSamplers[slot] = desc;
	m_SamplerValid[slot] = true;
}

ID3D12RootSignature* RootSignatureBuilder::Build()
{
	assert(m_ParameterValid.count() == m_Parameters.size());
	assert(m_SamplerValid.count() == m_StaticSamplers.size());

	return DXDevice::Instance().CreateRootSignature(m_Parameters, m_StaticSamplers);
}

void RootSignatureBuilder::SetDescriptor(D3D12_ROOT_PARAMETER_TYPE type, uint32_t slot, uint32_t reg, uint32_t space)
{
	D3D12_ROOT_PARAMETER1 param = {};
	param.ParameterType = type;
	param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	param.Descriptor.RegisterSpace = space;
	param.Descriptor.ShaderRegister = reg;

	if (m_Parameters.size() <= slot)
	{
		m_Parameters.resize(slot + 1ull);
	}

	m_Parameters[slot] = param;
	m_ParameterValid[slot] = true;
}
