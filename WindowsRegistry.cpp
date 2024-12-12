#include "WindowsError.h"
#include "WindowsRegistry.h"

namespace WindowsRegistry
{
	std::map<HKEY, const std::wstring> KeyHandleToKeyString =
	{
		{ HKEY_CLASSES_ROOT, L"HKEY_CLASSES_ROOT" },
		{ HKEY_CURRENT_CONFIG, L"HKEY_CURRENT_CONFIG" },
		{ HKEY_CURRENT_USER, L"HKEY_CURRENT_USER" },
		{ HKEY_LOCAL_MACHINE, L"HKEY_LOCAL_MACHINE" },
		{ HKEY_USERS, L"HKEY_USERS" },
	};

	std::map<std::wstring, const HKEY> KeyStringToKeyHandle =
	{
		{ L"HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT },
		{ L"HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG },
		{ L"HKEY_CURRENT_USER", HKEY_CURRENT_USER },
		{ L"HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE },
		{ L"HKEY_USERS", HKEY_USERS },
	};

	std::map<DWORD, const std::wstring> ValueTypeToValueTypeString =
	{
		{ REG_SZ, L"REG_SZ" },
		{ REG_EXPAND_SZ, L"REG_EXPAND_SZ" },
		{ REG_BINARY, L"REG_BINARY" },
		{ REG_DWORD, L"REG_DWORD" },
		{ REG_MULTI_SZ, L"REG_MULTI_SZ" },
		{ REG_QWORD, L"REG_QWORD" },
	};

	std::map<std::wstring, const DWORD> ValueTypeStringToValueType =
	{
		{ L"REG_SZ", REG_SZ },
		{ L"REG_EXPAND_SZ", REG_EXPAND_SZ },
		{ L"REG_BINARY", REG_BINARY },
		{ L"REG_DWORD", REG_DWORD },
		{ L"REG_MULTI_SZ", REG_MULTI_SZ },
		{ L"REG_QWORD", REG_QWORD },
	};

	std::optional<std::wstring> EnumKeys(HKEY hMainKey, LPCWSTR lpSubKey, const KeyNameProc& fnProcessKeyName)
	{
		HKEY hKey = NULL;

		LSTATUS lsCode = RegOpenKeyExW(hMainKey, lpSubKey, 0, KEY_ENUMERATE_SUB_KEYS, &hKey);

		if (ERROR_SUCCESS != lsCode)
		{
			return WindowsError::FormatErrorMessage(lsCode);
		}

		DWORD dwKeyIndex = 0;
		DWORD dwBufferSize = 256;
		LPWSTR lpKeyNameBuffer = new wchar_t[dwBufferSize];

		while (true)
		{
			DWORD dwLength = dwBufferSize;

			LSTATUS lsCode = RegEnumKeyExW(hKey, dwKeyIndex, lpKeyNameBuffer, &dwLength, NULL, NULL, NULL, NULL);

			if (ERROR_SUCCESS == lsCode)
			{
				std::shared_ptr<std::wstring> pKeyName = std::make_shared<std::wstring>(lpKeyNameBuffer);
				fnProcessKeyName(pKeyName);
				++dwKeyIndex;
			}
			else if (ERROR_NO_MORE_ITEMS == lsCode)
			{
				delete[] lpKeyNameBuffer;
				lpKeyNameBuffer = nullptr;
				break;
			}
			else if (ERROR_MORE_DATA == lsCode)
			{
				delete[] lpKeyNameBuffer;
				dwBufferSize *= 2;
				lpKeyNameBuffer = new wchar_t[dwBufferSize];
			}
			else
			{
				delete[] lpKeyNameBuffer;
				lpKeyNameBuffer = nullptr;
				RegCloseKey(hKey);
				return WindowsError::FormatErrorMessage(lsCode);
			}

		}

		RegCloseKey(hKey);

		return std::nullopt;
	}

	std::optional<std::wstring> EnumValues(HKEY hMainKey, LPCWSTR lpSubKey, const ValueProc& fnProcessValue)
	{
		HKEY hKey = NULL;

		LSTATUS lsCode = RegOpenKeyExW(hMainKey, lpSubKey, 0, KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hKey);

		if (ERROR_SUCCESS != lsCode)
		{
			return WindowsError::FormatErrorMessage(lsCode);
		}

		DWORD dwIndex = 0;
		DWORD dwNameBufferSize = 256;
		LPWSTR lpNameBuffer = new wchar_t[dwNameBufferSize];
		DWORD dwType = REG_NONE;
		DWORD dwDataBufferSize = 1024;
		LPBYTE lpDataBuffer = new BYTE[dwDataBufferSize];

		while (true)
		{
			DWORD dwNameLen = dwNameBufferSize;
			DWORD dwDataLen = dwDataBufferSize;

			LSTATUS lsCode = RegEnumValueW(hKey, dwIndex, lpNameBuffer, &dwNameLen, NULL, &dwType, lpDataBuffer, &dwDataLen);

			if (ERROR_SUCCESS == lsCode)
			{
				std::shared_ptr<RegistryValue> pRegValue = std::make_shared<RegistryValue>(lpNameBuffer, dwType);
				pRegValue->m_Data.resize(dwDataLen);
				std::copy(lpDataBuffer, lpDataBuffer + dwDataLen, pRegValue->m_Data.begin());
				fnProcessValue(pRegValue);
				++dwIndex;
			}
			else if (ERROR_NO_MORE_ITEMS == lsCode)
			{
				delete[] lpNameBuffer;
				delete[] lpDataBuffer;
				break;
			}
			else if (ERROR_MORE_DATA == lsCode)
			{
				if (dwNameBufferSize < dwNameLen)
				{
					delete[] lpNameBuffer;
					dwNameBufferSize *= 2;
					lpNameBuffer = new wchar_t[dwNameBufferSize];
				}

				if (dwDataBufferSize < dwDataLen)
				{
					delete[] lpDataBuffer;
					dwDataBufferSize *= 2;
					lpDataBuffer = new BYTE[dwDataBufferSize];
				}
			}
			else
			{
				std::wcerr << L"Failed to enumerate value, error code: " << lsCode << std::endl;
				delete[] lpNameBuffer;
				delete[] lpDataBuffer;
				RegCloseKey(hKey);
				return WindowsError::FormatErrorMessage(lsCode);
			}
		}

		RegCloseKey(hKey);

		return std::nullopt;
	}

	std::optional<std::wstring> CreateKey(HKEY hMainKey, LPCWSTR lpSubKey)
	{
		HKEY hKey = NULL;

		LSTATUS lsCode = RegCreateKeyExW(hMainKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);

		if (ERROR_SUCCESS != lsCode)
		{
			return WindowsError::FormatErrorMessage(lsCode);
		}

		RegCloseKey(hKey);

		return std::nullopt;
	}

	std::optional<std::wstring> DeleteKey(HKEY hMainKey, LPCWSTR lpSubKey)
	{
		LSTATUS lsCode = RegDeleteTreeW(hMainKey, lpSubKey);

		if (ERROR_SUCCESS != lsCode)
		{
			if (ERROR_FILE_NOT_FOUND == lsCode || ERROR_PATH_NOT_FOUND == lsCode)
			{
				// "Registry key not found, but this may not be an error."
				return std::nullopt;
			}
			else
			{
				return WindowsError::FormatErrorMessage(lsCode);
			}
		}

		return std::nullopt;
	}

	std::optional<std::wstring> SetValue(HKEY hMainKey, LPCWSTR lpSubKey, const RegistryValue& refRegValue)
	{
		if ((refRegValue.m_Name.empty()) && (REG_SZ != refRegValue.m_Type))
		{
			return std::wstring(L"键的默认Value类型必须为REG_SZ");
		}

		HKEY hKey = NULL;

		LSTATUS lsCode = RegCreateKeyExW(hMainKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL);

		if (ERROR_SUCCESS != lsCode)
		{
			return WindowsError::FormatErrorMessage(lsCode);
		}

		lsCode = RegSetValueExW(hKey,refRegValue.m_Name.c_str(),0,refRegValue.m_Type,refRegValue.m_Data.data(),static_cast<DWORD>(refRegValue.m_Data.size()));

		RegCloseKey(hKey);

		if (ERROR_SUCCESS != lsCode)
		{
			return WindowsError::FormatErrorMessage(lsCode);
		}

		return std::nullopt;
	}

	std::optional<std::wstring> DeleteValue(HKEY hMainKey, LPCWSTR lpSubKey, LPCWSTR lpValueName)
	{
		HKEY hKey = NULL;
		LSTATUS lsCode = RegOpenKeyExW(hMainKey, lpSubKey, 0, KEY_SET_VALUE | DELETE, &hKey);

		if (ERROR_SUCCESS != lsCode)
		{
			return WindowsError::FormatErrorMessage(lsCode);
		}

		lsCode = RegDeleteValueW(hKey, lpValueName);

		RegCloseKey(hKey);

		if (ERROR_SUCCESS != lsCode)
		{
			return WindowsError::FormatErrorMessage(lsCode);
		}

		return std::nullopt;
	}
	
	std::optional<std::wstring> QueryValue(RegistryValue& refOutRegValue, HKEY hMainKey, LPCWSTR lpSubKey, LPCWSTR lpValueName)
	{
		HKEY hKey = nullptr;

		LSTATUS lsCode = RegOpenKeyExW(hMainKey, lpSubKey, 0, KEY_READ, &hKey);
		if (ERROR_SUCCESS != lsCode)
		{
			return WindowsError::FormatErrorMessage(lsCode);
		}

		DWORD dwType = REG_NONE;
		DWORD dwSize = 0;
		lsCode = RegQueryValueExW(hKey, lpValueName, nullptr, &dwType, nullptr, &dwSize);
		if (ERROR_SUCCESS != lsCode)
		{
			RegCloseKey(hKey);
			return WindowsError::FormatErrorMessage(lsCode);
		}

		refOutRegValue.m_Name = lpValueName;
		refOutRegValue.m_Type = dwType;

		std::vector<BYTE> buffer(dwSize);
		lsCode = RegQueryValueExW(hKey, lpValueName, nullptr, &dwType, buffer.data(), &dwSize);
		if (lsCode == ERROR_SUCCESS) 
		{
			refOutRegValue.m_Data = buffer;
		}

		RegCloseKey(hKey);

		if (ERROR_SUCCESS != lsCode)
		{
			return WindowsError::FormatErrorMessage(lsCode);
		}

		return std::nullopt;
	}

	std::optional<std::wstring> RenameKey(HKEY hMainKey, LPCWSTR lpSubKey, LPCWSTR lpNewKeyName)
	{
		LSTATUS lsCode = RegRenameKey(hMainKey, lpSubKey, lpNewKeyName);

		if (ERROR_SUCCESS != lsCode)
		{
			return WindowsError::FormatErrorMessage(lsCode);
		}

		return std::nullopt;
	}

	std::optional<std::wstring> RenameValue(HKEY hMainKey, LPCWSTR lpSubKey, LPCWSTR lpCurValueName, LPCWSTR lpNewValueName)
	{
		HKEY hKey = NULL;
		LSTATUS lsCode;

		// 打开注册表键
		lsCode = RegOpenKeyExW(hMainKey, lpSubKey, 0, KEY_READ | KEY_SET_VALUE, &hKey);
		if (ERROR_SUCCESS != lsCode)
		{
			return WindowsError::FormatErrorMessage(lsCode);
		}

		// 检查新值名是否存在
		DWORD dwDummySize = 0;
		lsCode = RegQueryValueExW(hKey, lpNewValueName, NULL, NULL, NULL, &dwDummySize);
		if (ERROR_SUCCESS == lsCode)
		{
			// 重命名目标值失败，名称已存在
			RegCloseKey(hKey);
			return WindowsError::FormatErrorMessage(lsCode);
		}

		// 查询旧值的数据类型和大小
		DWORD dwType = REG_NONE;
		DWORD dwSize = 0;
		lsCode = RegQueryValueExW(hKey, lpCurValueName, NULL, &dwType, NULL, &dwSize);
		if (ERROR_SUCCESS != lsCode && ERROR_MORE_DATA != lsCode)
		{
			RegCloseKey(hKey);
			return WindowsError::FormatErrorMessage(lsCode);
		}

		// 读取旧值的数据
		std::vector<BYTE> ValueData(dwSize);
		lsCode = RegQueryValueExW(hKey, lpCurValueName, NULL, &dwType, ValueData.data(), &dwSize);
		if (ERROR_SUCCESS != lsCode)
		{
			RegCloseKey(hKey);
			return WindowsError::FormatErrorMessage(lsCode);
		}

		// 设置新值
		lsCode = RegSetValueExW(hKey, lpNewValueName, 0, dwType, ValueData.data(), static_cast<DWORD>(ValueData.size()));
		if (ERROR_SUCCESS != lsCode)
		{
			RegCloseKey(hKey);
			return WindowsError::FormatErrorMessage(lsCode);
		}

		// 删除旧值
		lsCode = RegDeleteValueW(hKey, lpCurValueName);
		if (ERROR_SUCCESS != lsCode)
		{
			RegCloseKey(hKey);
			return WindowsError::FormatErrorMessage(lsCode);
		}

		RegCloseKey(hKey);

		return std::nullopt;
	}
}
