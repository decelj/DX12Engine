#include "stdafx.h"
#include "RootSignatureBuilder.h"
#include "DXDevice.h"


RootSignatureBuilder::RootSignatureBuilder()
{
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
		m_Parameters.resize(slot + 1u);
	}

	m_Parameters[slot] = param;
	m_Valid[slot] = true;
}

ID3D12RootSignature* RootSignatureBuilder::Build()
{
	assert(m_Valid.count() == m_Parameters.size());

	return DXDevice::Instance().CreateRootSignature(m_Parameters);
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
		m_Parameters.resize(slot + 1u);
	}

	m_Parameters[slot] = param;
	m_Valid[slot] = true;
}
