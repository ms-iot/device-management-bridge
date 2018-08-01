/*
Copyright 2018 Microsoft
Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "stdafx.h"
#include "RegistryUtils.h"
#include "DMBridgeException.h"
#include "Logger.h"

using namespace std;

namespace Utils
{
	void WriteRegistryValue(const wstring& subKey, const wstring& propName, const wstring& propValue)
	{
		LSTATUS status;
		HKEY hKey = NULL;
		status = RegCreateKeyEx(
			HKEY_LOCAL_MACHINE,
			subKey.c_str(),
			0,      // reserved
			NULL,   // user-defined class type of this key.
			0,      // default; non-volatile
			KEY_ALL_ACCESS,
			NULL,   // inherit security descriptor from parent.
			&hKey,
			NULL    // disposition [optional, out]
		);
		if (status != ERROR_SUCCESS) {
			throw DMBridgeExceptionWithErrorCode(status);
		}

		status = RegSetValueEx(hKey, propName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(propValue.c_str()), (static_cast<unsigned int>(propValue.size()) + 1) * sizeof(propValue[0]));
		if (status != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			throw DMBridgeExceptionWithErrorCode(status);
		}

		RegCloseKey(hKey);
	}

	void WriteRegistryValue(const wstring& subKey, const wstring& propName, unsigned long propValue)
	{
		LSTATUS status;
		HKEY hKey = NULL;
		status = RegCreateKeyEx(
			HKEY_LOCAL_MACHINE,
			subKey.c_str(),
			0,      // reserved
			NULL,   // user-defined class type of this key.
			0,      // default; non-volatile
			KEY_ALL_ACCESS,
			NULL,   // inherit security descriptor from parent.
			&hKey,
			NULL    // disposition [optional, out]
		);
		if (status != ERROR_SUCCESS) {
			throw DMBridgeExceptionWithErrorCode(status);
		}

		status = RegSetValueEx(hKey, propName.c_str(), 0, REG_DWORD, reinterpret_cast<BYTE*>(&propValue), sizeof(propValue));
		if (status != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			throw DMBridgeExceptionWithErrorCode(status);
		}

		RegCloseKey(hKey);
	}

	bool RegistryKeyExists(const wstring& subKey)
	{
		HKEY keyHandle = NULL;
		LONG result = RegOpenKey(HKEY_LOCAL_MACHINE, subKey.c_str(), &keyHandle);
		if (result == ERROR_SUCCESS)
		{
			RegCloseKey(keyHandle);
			return true;
		}
		return false;
	}

	LSTATUS TryReadRegistryValue(const wstring& subKey, const wstring& propName, wstring& propValue)
	{
		DWORD dataSize = 0;
		LSTATUS status;
		status = RegGetValue(HKEY_LOCAL_MACHINE, subKey.c_str(), propName.c_str(), RRF_RT_REG_SZ, NULL, NULL, &dataSize);
		if (status != ERROR_SUCCESS)
		{
			return status;
		}

		vector<char> data(dataSize);
		status = RegGetValue(HKEY_LOCAL_MACHINE, subKey.c_str(), propName.c_str(), RRF_RT_REG_SZ, NULL, data.data(), &dataSize);
		if (status != ERROR_SUCCESS)
		{
			return status;
		}

		propValue = reinterpret_cast<const wchar_t*>(data.data());

		return ERROR_SUCCESS;
	}

	LSTATUS TryReadRegistryValue(const wstring& subKey, const wstring& propName, unsigned long& propValue)
	{
		DWORD dataSize = 0;
		LSTATUS status;
		status = RegGetValue(HKEY_LOCAL_MACHINE, subKey.c_str(), propName.c_str(), RRF_RT_REG_DWORD, NULL, NULL, &dataSize);
		if (status != ERROR_SUCCESS)
		{
			return status;
		}

		vector<char> data(dataSize);
		status = RegGetValue(HKEY_LOCAL_MACHINE, subKey.c_str(), propName.c_str(), RRF_RT_REG_DWORD, NULL, data.data(), &dataSize);
		if (status != ERROR_SUCCESS)
		{
			return status;
		}

		propValue = *(reinterpret_cast<const unsigned long*>(data.data()));

		return ERROR_SUCCESS;
	}

	wstring ReadRegistryValue(const wstring& subKey, const wstring& propName)
	{
		wstring propValue;
		LSTATUS status = TryReadRegistryValue(subKey, propName, propValue);
		if (status != ERROR_SUCCESS)
		{
			TRACEP(L"Error: Could not read registry value: ", (subKey + L"\\" + propName).c_str());
			throw DMBridgeExceptionWithErrorCode(status);
		}
		return propValue;
	}

	wstring ReadRegistryValue(const wstring& subKey, const wstring& propName, const wstring& propDefaultValue)
	{
		wstring propValue;
		if (ERROR_SUCCESS != Utils::TryReadRegistryValue(subKey, propName, propValue))
		{
			propValue = propDefaultValue;
		}
		return propValue;
	}
}
