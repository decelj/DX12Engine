#pragma once

#include "stdafx.h"

std::wstring ToWStr(const char* str);
void PrintLastErrorMessage(const wchar_t* functionName);

inline std::string HrToString(HRESULT hr)
{
	char s_str[64] = {};

	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}
