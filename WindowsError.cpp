#include "WindowsError.h"

namespace WindowsError
{

	std::wstring FormatErrorMessage(DWORD dwErrorCode)
	{
		DWORD dwSize = 256;
		std::vector<wchar_t> vMsgBuffer(dwSize);

		DWORD dwMsgLen = FormatMessageW(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			dwErrorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			vMsgBuffer.data(),
			static_cast<DWORD>(vMsgBuffer.size()),
			nullptr
		);

		if (dwMsgLen == 0)
		{
			return L"Unknown error code.";
		}

		if (dwMsgLen > 0 && dwMsgLen < dwSize)
		{
			vMsgBuffer[dwMsgLen] = L'\0';
		}
		else
		{
			// 如果缓冲区不够大，重新分配并再次尝试
			vMsgBuffer.resize(dwMsgLen + 1);
			dwMsgLen = FormatMessageW(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				dwErrorCode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				vMsgBuffer.data(),
				static_cast<DWORD>(vMsgBuffer.size()),
				nullptr
			);

			if (dwMsgLen == 0)
			{
				return L"Unknown error code.";
			}
		}

		return std::wstring(vMsgBuffer.data());
	}
}