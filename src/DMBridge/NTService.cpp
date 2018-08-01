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
#include "NTService.h"
#include "Logger.h"
#include "ServiceManager.h"
#include "DMBridgeException.h"

// https://msdn.microsoft.com/en-us/library/ms682450(VS.85).aspx
constexpr int MaxServiceNameLength = 256;
std::unique_ptr<NTServiceConfig> NTService::_config;

using namespace std;

/* Map Generated rpc method signatures to class */
/* -------------------------------------------- */
HRESULT StartServiceRpc(_In_ handle_t, _In_ wchar_t *serviceName)
{
	return NTService::Start(serviceName);
}
HRESULT StopServiceRpc(_In_ handle_t, _In_ wchar_t *serviceName)
{
	return NTService::Stop(serviceName);
}
HRESULT QueryServiceRpc(_In_ handle_t, _In_ wchar_t* serviceName, _Outptr_ INT32* status)
{
	return NTService::Query(serviceName, status);
}
/* -------------------------------------------- */

HRESULT NTService::Start(_In_ const wstring& serviceName)
{
	TRACE(__FUNCTION__);
	TRACEP(L"Start request for: ", serviceName);
	HRESULT result = ValidateNameArgument(serviceName, true);
	if (result != S_OK)
		return result;

	try
	{
		TRACE("Starting...");
		ServiceManager::Start(serviceName);
	}
	catch (const DMBridgeExceptionWithErrorCode& e)
	{
		TRACEP("Failed to start service. Exception caught. Error: ", e.ErrorCode());
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
		TRACEP("Failed to start service. Unknown exception caught. Error: ", lastError);
		// Prevent a 0'd last error from returning S_OK
		if (lastError == 0)
		{
			return E_FAIL;
		}
		return HRESULT_FROM_WIN32(lastError);
	}

	return S_OK;
}

HRESULT NTService::Stop(_In_ const wstring& serviceName)
{
	TRACE(__FUNCTION__);
	TRACEP(L"Stop request for: ", serviceName);
	HRESULT result = ValidateNameArgument(serviceName, true);
	if (result != S_OK)
		return result;

	try
	{
		TRACE("Stopping...");
		ServiceManager::Stop(serviceName);
	}
	catch (const DMBridgeExceptionWithErrorCode& e)
	{
		TRACEP("Failed to stop service. Exception caught. Error: ", e.ErrorCode());
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
		TRACEP("Failed to sotp service. Unknown exception caught. Error ", lastError);
		// Prevent a 0'd last error from returning S_OK
		if (lastError == 0)
		{
			return E_FAIL;
		}
		return HRESULT_FROM_WIN32(lastError);
	}

	return S_OK;
}

HRESULT NTService::Query(_In_ const wstring& serviceName, _Outptr_ INT32* status)
{
	TRACE(__FUNCTION__);
	TRACEP(L"Query request for: ", serviceName);
	HRESULT result = ValidateNameArgument(serviceName, false);
	if (result != S_OK)
		return result;

	if (status == nullptr)
	{
		TRACE("status is nullptr");
		return HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);
	}

	try
	{
		DWORD serviceStatus = ServiceManager::GetStatus(serviceName);
		INT32 intStatus = static_cast<INT32>(serviceStatus);
		*status = intStatus;
	}
	catch (const DMBridgeExceptionWithErrorCode& e)
	{
		TRACEP("Failed to query service. Exception caught. Error: ", e.ErrorCode());
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
		TRACEP("Failed to query service. Unknown exception caught. Error: ", lastError);
		// Prevent a 0'd last error from returning S_OK
		if (lastError == 0)
		{
			return E_FAIL;
		}
		return HRESULT_FROM_WIN32(lastError);
	}

	return S_OK;
}

HRESULT NTService::ValidateNameArgument(_In_ const wstring& serviceName, _In_ const bool enforceWhitelist)
{
	TRACE(__FUNCTION__);

	TRACEP("Enforcing whitelist: ", enforceWhitelist);

	size_t nameLength = serviceName.length();
	if (nameLength == 0 || nameLength > MaxServiceNameLength)
	{
		TRACE("serviceName failed length check");
		return HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME);
	}

	if (!IsValidName(serviceName.c_str()))
	{
		TRACE("serviceName failed regex check");
		return HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME);
	}

	if (enforceWhitelist && !IsWhitelisted(serviceName.c_str()))
	{
		TRACE("serviceName is not on whitelist");
		return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
	}

	return S_OK;
}

bool NTService::IsWhitelisted(_In_ const wstring& serviceName)
{
	TRACE(__FUNCTION__);

	if (_config == nullptr)
	{
		TRACE("NTServiceConfig is null");
		return false;
	}
	auto whitelist = _config->GetWhitelist();
	return (whitelist.find(serviceName) != whitelist.end());
}

/*
* https://msdn.microsoft.com/en-us/library/system.serviceprocess.serviceinstaller.servicename(v=vs.110).aspx
*  Service names cannot contain forward or backward slashes,
*  or characters from the ASCII character set with value less than decimal value 32.
*/
bool NTService::IsValidName(_In_ const wstring& serviceName)
{
	TRACE(__FUNCTION__);
	// Start at x01 as x00 is an invalid sequence
	wregex namePattern(L"^[^\x01-\x1F\/\\\\]+$");
	wcmatch match;
	return (regex_match(serviceName.c_str(), match, namePattern));
}