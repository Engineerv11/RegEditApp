#pragma once
#include <sstream>
#include <iomanip>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include "WindowsRegistry.h"

HKEY ExtractMainKey(std::wstring strKey);

std::wstring ExtractSubKey(std::wstring strKey);

std::wstring GetSubKey(const QTreeWidgetItem* pItem);

HKEY GetMainKey(const QTreeWidgetItem* pItem);

bool ExistSubkeys(HKEY hMainKey, LPCWSTR lpSubKey);

std::wstring DWORDToHexString(DWORD dwValue);

DWORD HexStringToDWORD(const std::wstring& strHex);

std::wstring RVDataToString(DWORD dwType, const std::vector<BYTE>& refData);

std::vector<BYTE> RVDataFromString(DWORD dwType, const std::wstring& refDataStr);