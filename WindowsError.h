#pragma once
#include <vector>
#include <string>
#include <Windows.h>

namespace WindowsError
{
	std::wstring FormatErrorMessage(DWORD dwErrorCode);
}