#include "Kits.h"

HKEY ExtractMainKey(std::wstring strKey)
{
    if (strKey.empty()) 
    {
        return 0; 
    }

    return WindowsRegistry::KeyStringToKeyHandle[strKey.substr(0, strKey.find(L'\\'))];
}

std::wstring ExtractSubKey(std::wstring strKey)
{
    if (strKey.empty())
    {
        return 0;
    }

    return strKey.substr(strKey.find_first_of(L'\\') + 1);
}

std::wstring GetSubKey(const QTreeWidgetItem* pItem)
{
    QStringList slKeyLevel;
    while (pItem->parent())
    {
        slKeyLevel.prepend(pItem->text(0));
        pItem = pItem->parent();
    }
    return slKeyLevel.join("\\").toStdWString();
}

HKEY GetMainKey(const QTreeWidgetItem* pItem)
{
    while (pItem->parent())
    {
        pItem = pItem->parent();
    }

    return WindowsRegistry::KeyStringToKeyHandle[pItem->text(0).toStdWString()];
}

bool ExistSubkeys(HKEY hMainKey, LPCWSTR lpSubKey)
{
    HKEY hKey = NULL;
    LSTATUS lsCode = RegOpenKeyExW(hMainKey, lpSubKey, 0, KEY_READ, &hKey);
    if (ERROR_SUCCESS != lsCode)
    {
        return false;
    }

    DWORD cSubKeys = 0;
    DWORD cchMaxSubKeyLen = 0;
    lsCode = RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &cSubKeys, &cchMaxSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL);
    if (ERROR_SUCCESS != lsCode)
    {
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return cSubKeys > 0;
}

std::wstring DWORDToHexString(DWORD dwValue)
{
    std::wstringstream wss;
    wss << std::hex << std::setw(8) << std::setfill(L'0') << dwValue;
    return wss.str();
}

DWORD HexStringToDWORD(const std::wstring& strHex)
{
    std::wistringstream wiss(strHex);
    DWORD dwValue;
    wiss >> std::hex >> dwValue;
    return dwValue;
}

std::wstring RVDataToString(DWORD dwType, const std::vector<BYTE>& refData)
{
    if (!refData.empty())
    {
        if (REG_SZ == dwType)
        {
            return std::wstring(reinterpret_cast<const wchar_t*>(refData.data()));
        }
        else if (REG_DWORD == dwType)
        {
            return DWORDToHexString(*reinterpret_cast<const DWORD*>(refData.data()));
        }
    }

    return std::wstring();
}

std::vector<BYTE> RVDataFromString(DWORD dwType, const std::wstring& refDataStr)
{
    if (!refDataStr.empty())
    {
        if (REG_SZ == dwType)
        {
            std::vector<BYTE> vData((refDataStr.size() + 1) * sizeof(wchar_t));
            std::memcpy(vData.data(), refDataStr.data(), vData.size());
            return vData;
        }
        else if (REG_DWORD == dwType)
        {
            DWORD dwData = HexStringToDWORD(refDataStr);
            std::vector<BYTE> vData(sizeof(DWORD));
            std::memcpy(vData.data(), &dwData, vData.size());
            return vData;
        }
    }

    return std::vector<BYTE>();
}
