#include"stdafx.h"
#include "WindowsHelpers.h"

#include <string>
#include <locale>
#include <codecvt>

std::wstring ToWStr(const char* str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return converter.from_bytes(str);
}

void PrintLastErrorMessage(const wchar_t* functionName)
{
	constexpr uint32_t kBufferSize = 1028u;

	WCHAR messageBuffer[kBufferSize];
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		messageBuffer,
		kBufferSize,
		nullptr);

	std::wstring message = L"Error while calling ";
	message += functionName;
	message += L": ";
	message += messageBuffer;

	OutputDebugStringW(message.c_str());
}

