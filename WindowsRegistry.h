#pragma once
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <iostream>
#include <optional>
#include <functional>
#include <Windows.h>

namespace WindowsRegistry
{
	extern std::map<HKEY, const std::wstring> KeyHandleToKeyString;

	extern std::map<std::wstring, const HKEY> KeyStringToKeyHandle;
	
	extern std::map<DWORD, const std::wstring> ValueTypeToValueTypeString;

	extern std::map<std::wstring, const DWORD> ValueTypeStringToValueType;

	struct RegistryValue
	{
		RegistryValue() = default;
		RegistryValue(const wchar_t* lpName, DWORD dwType = REG_NONE) :m_Name(lpName),m_Type(dwType) {}
		std::wstring m_Name;
		DWORD m_Type;
		std::vector<BYTE> m_Data;
	};

	using KeyNameProc = std::function<void(std::shared_ptr<std::wstring>)>;

	std::optional<std::wstring> EnumKeys(HKEY hMainKey, LPCWSTR lpSubKey, const KeyNameProc& fnProcessKeyName);

    using ValueProc = std::function<void(std::shared_ptr<RegistryValue>)>;

	std::optional<std::wstring> EnumValues(HKEY hMainKey, LPCWSTR lpSubKey, const ValueProc& fnProcessValue);

	std::optional<std::wstring> CreateKey(HKEY hMainKey, LPCWSTR lpSubKey);

	std::optional<std::wstring> DeleteKey(HKEY hMainKey, LPCWSTR lpSubKey);

	std::optional<std::wstring> SetValue(HKEY hMainKey, LPCWSTR lpSubKey, const RegistryValue& refRegValue);

	std::optional<std::wstring> DeleteValue(HKEY hMainKey, LPCWSTR lpSubKey, LPCWSTR lpValueName);

	std::optional<std::wstring> QueryValue(RegistryValue& refOutRegValue, HKEY hMainKey, LPCWSTR lpSubKey, LPCWSTR lpValueName);

	std::optional<std::wstring> RenameKey(HKEY hMainKey, LPCWSTR lpSubKey, LPCWSTR lpNewKeyName);

	std::optional<std::wstring> RenameValue(HKEY hMainKey, LPCWSTR lpSubKey, LPCWSTR lpCurValueName, LPCWSTR lpNewValueName);
}
