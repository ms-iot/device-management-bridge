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
#include "TelemetryLevel.h"
#include "DMBridgeException.h"
#include "Logger.h"
#include "RegistryUtils.h"

constexpr int MinTelemetryLevel = 0;
constexpr int MaxTelemetryLevel = 3;
constexpr wchar_t* TelemetryLevelKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\DataCollection";
constexpr wchar_t* TelemetryLevelValue = L"AllowTelemetry";

using namespace std;

// Define the private mutex
mutex TelemetryLevel::_setLevelMutex;

/* Map generated rpc method signatures to class */
/* -------------------------------------------- */
HRESULT SetTelemetryLevelRpc(_In_ handle_t, _In_ INT32 level)
{
	return TelemetryLevel::Set(level);
}
HRESULT GetTelemetryLevelRpc(_In_ handle_t, _Outptr_ INT32 *level)
{
	return TelemetryLevel::Get(level);
}
/* -------------------------------------------- */

HRESULT TelemetryLevel::Set(_In_ const INT32 level)
{
	TRACE(__FUNCTION__);
	TRACEP("Setting telemetry level to: ", level);

	if (level < MinTelemetryLevel || level > MaxTelemetryLevel)
	{
		TRACE("Invalid level");
		return HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);
	}

	try
	{
		const DWORD dwLevel = static_cast<DWORD>(level);

		TRACE("Aquiring lock");
		lock_guard<mutex> lock(_setLevelMutex);
		TRACE("Lock aquired");
		Utils::WriteRegistryValue(TelemetryLevelKey, TelemetryLevelValue, dwLevel);
	}
	catch (const DMBridgeExceptionWithErrorCode& e)
	{
		TRACEP("Failed set telemetry level. Exception caught. Error: ", e.ErrorCode());
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
		TRACEP("Failed to set telemetry level. Unknown exception caught. Error: ", lastError);
		// Prevent a 0'd last error from returning S_OK
		if (lastError == 0)
		{
			return E_FAIL;
		}
		return HRESULT_FROM_WIN32(lastError);
	}
	return S_OK;
}

HRESULT TelemetryLevel::Get(_Outptr_ INT32* level)
{
	TRACE(__FUNCTION__);

	if (level == nullptr)
	{
		TRACE("level is nullptr");
		return HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);
	}

	try
	{
		DWORD dwLevel = 0;
		Utils::TryReadRegistryValue(TelemetryLevelKey, TelemetryLevelValue, dwLevel);
		*level = static_cast<INT32>(dwLevel);
	}
	catch (const DMBridgeExceptionWithErrorCode& e)
	{
		TRACEP("Failed get telemetry level. Exception caught. Error: ", e.ErrorCode());
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
		TRACEP("Failed to get telemetry level. Unknown exception caught. Error: ", lastError);
		// Prevent a 0'd last error from returning S_OK
		if (lastError == 0)
		{
			return E_FAIL;
		}
		return HRESULT_FROM_WIN32(lastError);
	}

	return S_OK;
}