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
#include "ComputerName.h"
#include "DMBridgeException.h"
#include "Logger.h"
#include "RegistryUtils.h"

constexpr wchar_t* TcpNameKey = L"system\\currentcontrolset\\services\\tcpip\\parameters";
constexpr wchar_t* TcpNameValue = L"NV HostName";
constexpr wchar_t* CompNameKey1 = L"system\\currentcontrolset\\control\\computername\\computername";
constexpr wchar_t* CompNameKey2 = L"system\\currentcontrolset\\control\\computername\\activecomputername";
constexpr wchar_t* CompNameValue = L"ComputerName";

constexpr wchar_t* WhiteSpaceChars = L"\t\n ";

using namespace std;

// Define the private mutex
mutex ComputerName::_setNameMutex;

/* Map Generated rpc method signatures to class */
/* -------------------------------------------- */
HRESULT SetComputerNameRpc(_In_ handle_t, _In_ const wchar_t *computerName)
{
	return ComputerName::Set(computerName);
}
HRESULT GetComputerNameRpc(_In_ handle_t, _Outptr_ long *size, _Outptr_ wchar_t **computerName)
{
	return ComputerName::Get(*size, *computerName);
}
HRESULT IsComputerRenamePendingRpc(_In_ handle_t, _Outptr_ BOOL* isPending)
{
	return ComputerName::IsRenamePending(isPending);
}
/* -------------------------------------------- */

HRESULT ComputerName::Set(_In_ const wstring& computerName)
{
	TRACE(__FUNCTION__);

	TRACEP(L"Request to set computer name to: ", computerName);
	size_t nameLength = computerName.length();
	if (nameLength == 0 || nameLength > MAX_COMPUTERNAME_LENGTH)
	{
		TRACE("Invalid computerName length");
		return HRESULT_FROM_WIN32(ERROR_INVALID_COMPUTERNAME);
	}

	if (!IsValidName(computerName))
	{
		TRACE("Invalid computerName");
		return HRESULT_FROM_WIN32(ERROR_INVALID_COMPUTERNAME);
	}

	try
	{
		long result = UpdateNameInRegistry(computerName);
		if (result != ERROR_SUCCESS)
		{
			TRACEP("Failed to set computer name. Error: ", result);
			return HRESULT_FROM_WIN32(result);
		}
	}
	catch (...)
	{
		DWORD lastError = GetLastError();
		TRACEP("Failed to set computer name. Unknown exception caught. Error: ", lastError);
		// Prevent a 0'd last error from returning S_OK
		if (lastError == 0)
		{
			return E_FAIL;
		}
		return HRESULT_FROM_WIN32(lastError);
	}

	return S_OK;
}

HRESULT ComputerName::Get(_Outptr_ long &size, _Outptr_ wchar_t *&computerName)
{
	TRACE(__FUNCTION__);

	try
	{
		wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
		size = sizeof(buffer) / sizeof(wchar_t);
		DWORD dwSize = static_cast<DWORD>(size);

		if (!GetComputerNameEx(ComputerNamePhysicalNetBIOS, buffer, &dwSize))
		{
			DWORD lastError = GetLastError();
			TRACEP("Failed to GetComputerNameEx(). Error: ", lastError);
			return HRESULT_FROM_WIN32(lastError);
		}

		computerName = (wchar_t*)midl_user_allocate(size * sizeof(wchar_t));
		if (computerName == NULL)
		{
			TRACE("Failed to get computer name. Could not allocate memory.");
			return E_OUTOFMEMORY;
		}

		errno_t copyErr = wcscpy_s(computerName, size, (const wchar_t*)(buffer));
		if (copyErr != 0)
		{
			TRACEP("Failed to get computer name. Could not copy buffer to out pointer. Errno: ", copyErr);
			return E_FAIL;
		}
	}
	catch (...)
	{
		DWORD lastError = GetLastError();
		TRACEP("Failed to get computer name. Unknown exception caught. Error: ", lastError);
		// Prevent a 0'd last error from returning S_OK
		if (lastError == 0)
		{
			return E_FAIL;
		}
		return HRESULT_FROM_WIN32(lastError);
	}

	return S_OK;
}

HRESULT ComputerName::IsRenamePending(_Outptr_ BOOL* isPending)
{
	TRACE(__FUNCTION__);

	if (isPending == nullptr)
	{
		TRACE("isPending is nullptr");
		return HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);
	}

	// Treat this method as an atomic operation
	TRACE("Aquiring lock");
	lock_guard<mutex> lock(_setNameMutex);
	TRACE("Lock aquired");

	*isPending = false;

	try
	{
		wstring pendingName;
		wstring activeName;

		if (Utils::TryReadRegistryValue(CompNameKey1, CompNameValue, pendingName) != ERROR_SUCCESS)
		{
			DWORD lastError = GetLastError();
			TRACEP("Failed to get registry computer name. Error: ", lastError);
			return HRESULT_FROM_WIN32(lastError);
		}

		wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
		size_t dwSize_t = sizeof(buffer) / sizeof(wchar_t);
		DWORD dwSize = static_cast<DWORD>(dwSize_t);

		if (!GetComputerNameEx(ComputerNamePhysicalNetBIOS, buffer, &dwSize))
		{
			DWORD lastError = GetLastError();
			TRACEP("Failed to GetComputerNameEx(). Error: ", lastError);
			return HRESULT_FROM_WIN32(lastError);
		}

		activeName = wstring(buffer);

		if (_wcsicmp(pendingName.c_str(), activeName.c_str()) != 0)
		{
			TRACE("Name change is pending");
			*isPending = true;
		}
	}
	catch (const DMBridgeExceptionWithErrorCode& e)
	{
		TRACEP("Failed to check if rename pending. Exception caught. Error: ", e.ErrorCode());
		// Prevent a 0'd error from returning S_OK
		if (e.ErrorCode() == 0)
		{
			return E_FAIL;
		}
		return HRESULT_FROM_WIN32(e.ErrorCode());
	}
	catch (...)
	{
		DWORD lastError = GetLastError();
		TRACEP("Failed to check if rename pending. Unknown exception caught. Error: ", lastError);
		// Prevent a 0'd last error from returning S_OK
		if (lastError == 0)
		{
			return E_FAIL;
		}
		return HRESULT_FROM_WIN32(lastError);
	}

	return S_OK;
}

/*
* Calling SetComputerNameEx currently fails to update the NetBIOS name on IoT core.
* Instead, manually update the registry. *NOTE*: This will cause domain trust issues
* if the computer is bound to a domain. There are specific Win32 APIs for that scenario.
*/
long ComputerName::UpdateNameInRegistry(_In_ const wstring& computerName)
{
	TRACE(__FUNCTION__);

	// Treat this method as an atomic operation
	TRACE("Aquiring lock");
	lock_guard<mutex> lock(_setNameMutex);
	TRACE("Lock aquired");
	TRACEP(L"Setting computer name to: ", computerName);

	const wchar_t* wcharComputerName = computerName.c_str();

	size_t len = (wcslen(wcharComputerName) + 1) * sizeof(wchar_t);
	DWORD dwLen = static_cast<DWORD>(len);

	long result = ERROR_SUCCESS;
	result = RegSetKeyValue(HKEY_LOCAL_MACHINE, CompNameKey1, CompNameValue, REG_SZ, (LPBYTE)wcharComputerName, dwLen);
	if (result != ERROR_SUCCESS)
	{
		TRACEP(L"Failed to update registry key: ", CompNameKey1);
		return result;
	}

	result = RegSetKeyValue(HKEY_LOCAL_MACHINE, CompNameKey2, CompNameValue, REG_SZ, (LPBYTE)wcharComputerName, dwLen);
	if (result != ERROR_SUCCESS)
	{
		TRACEP(L"Failed to update registry key: ", CompNameKey2);
		return result;
	}

	result = RegSetKeyValue(HKEY_LOCAL_MACHINE, TcpNameKey, TcpNameValue, REG_SZ, (LPBYTE)wcharComputerName, dwLen);
	if (result != ERROR_SUCCESS)
	{
		TRACEP(L"Failed to update registry key: ", TcpNameKey);
		return result;
	}

	return ERROR_SUCCESS;
}

/*
*  https://docs.microsoft.com/en-us/windows/desktop/SysInfo/computer-names
*  NetBIOS names consist of up to 15 bytes of OEM characters including letters, digits, hyphens, and periods.
*/
bool ComputerName::IsValidName(_In_ const wstring& computerName)
{
	TRACE(__FUNCTION__);

	wregex allowedCharacters(L"^[a-zA-Z0-9\-.]+$");
	wcmatch match;
	if (!(regex_match(computerName.c_str(), match, allowedCharacters)))
	{
		return false;
	}

	wregex allDigits(L"^[0-9]+$");
	if ((regex_match(computerName.c_str(), match, allDigits)))
	{
		return false;
	}

	return true;
}