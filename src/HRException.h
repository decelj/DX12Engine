#pragma once

#include "stdafx.h"
#include "WindowsHelpers.h"

#include <stdexcept>

class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr)
		: std::runtime_error(HrToString(hr))
		, m_HR(hr)
	{}

	HRESULT Error() const { return m_HR; }

private:
	const HRESULT m_HR;
};

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw HrException(hr);
	}
}
