#pragma once

#include "stdafx.h"

#include <bitset>
#include <d3d12.h>

class DXDevice;

class RootSignatureBuilder
{
public:
	RootSignatureBuilder();
	~RootSignatureBuilder() = default;

	void SetSRV(uint32_t slot, uint32_t reg, uint32_t space) { SetDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, slot, reg, space); }
	void SetUAV(uint32_t slot, uint32_t reg, uint32_t space) { SetDescriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, slot, reg, space); }
	void SetCBV(uint32_t slot, uint32_t reg, uint32_t space) { SetDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, slot, reg, space); }
	void SetConstant(uint32_t slot, uint32_t reg, uint32_t space, uint32_t count);

	ID3D12RootSignature* Build(DXDevice& device);

private:
	void SetDescriptor(D3D12_ROOT_PARAMETER_TYPE type, uint32_t slot, uint32_t reg, uint32_t space);

	std::vector<D3D12_ROOT_PARAMETER1>	m_Parameters;
	std::bitset<32u>					m_Valid;
};
