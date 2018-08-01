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
#include <functional>
#include <vector>
#include "ServiceManager.h"
#include "Logger.h"
#include "AutoCloseHandle.h"
#include "DMBridgeException.h"

using namespace std;

constexpr int SERVICE_WAIT_CYCLE_MS = 1000;

DWORD ServiceManager::GetStatus(const wstring& serviceName)
{
	TRACE(__FUNCTION__);

	TRACEP(L"Checking the running state of service: ", serviceName.c_str());

	Utils::AutoCloseServiceHandle serviceManagerHandle = OpenSCManager(NULL /*local machine*/, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
	if (serviceManagerHandle.Get() == NULL)
	{
		throw DMBridgeExceptionWithErrorCode("OpenSCManager() failed.", GetLastError());
	}

	Utils::AutoCloseServiceHandle serviceHandle = OpenService(serviceManagerHandle.Get() /*scm manager*/, serviceName.c_str(), SERVICE_ALL_ACCESS);
	if (serviceHandle.Get() == NULL)
	{
		throw DMBridgeExceptionWithErrorCode("OpenService() failed.", GetLastError());
	}

	SERVICE_STATUS serviceStatus;
	if (!QueryServiceStatus(serviceHandle.Get() /*service handle*/, &serviceStatus))
	{
		throw DMBridgeExceptionWithErrorCode("QueryServiceStatus() failed.", GetLastError());
	}

	return serviceStatus.dwCurrentState;
}

void ServiceManager::WaitStatus(const wstring& serviceName, DWORD status, unsigned int maxWaitInSeconds)
{
	while (maxWaitInSeconds--)
	{
		if (status == ServiceManager::GetStatus(serviceName))
		{
			break;
		}
		TRACEP(L"Waiting for service: ", serviceName.c_str());
		::Sleep(SERVICE_WAIT_CYCLE_MS);
		--maxWaitInSeconds;
	}
}

DWORD ServiceManager::GetStartType(const wstring& serviceName)
{
	TRACE(__FUNCTION__);

	TRACEP(L"Checking the enabled state of service: ", serviceName.c_str());

	Utils::AutoCloseServiceHandle serviceManagerHandle = OpenSCManager(NULL /*local machine*/, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
	if (serviceManagerHandle.Get() == NULL)
	{
		throw DMBridgeExceptionWithErrorCode("OpenSCManager() failed.", GetLastError());
	}

	Utils::AutoCloseServiceHandle serviceHandle = OpenService(serviceManagerHandle.Get() /*scm manager*/, serviceName.c_str(), SERVICE_ALL_ACCESS);
	if (serviceHandle.Get() == NULL)
	{
		throw DMBridgeExceptionWithErrorCode("OpenService() failed.", GetLastError());
	}

	DWORD bytesNeeded = 0;
	if (!QueryServiceConfig(serviceHandle.Get(), NULL, 0, &bytesNeeded) && (ERROR_INSUFFICIENT_BUFFER != GetLastError()))
	{
		throw DMBridgeExceptionWithErrorCode("QueryServiceConfig() failed.", GetLastError());
	}

	vector<char> buffer(bytesNeeded);
	if (!QueryServiceConfig(serviceHandle.Get(), reinterpret_cast<QUERY_SERVICE_CONFIG*>(buffer.data()), static_cast<DWORD>(buffer.size()), &bytesNeeded))
	{
		throw DMBridgeExceptionWithErrorCode("QueryServiceConfig() failed.", GetLastError());
	}

	QUERY_SERVICE_CONFIG* config = reinterpret_cast<QUERY_SERVICE_CONFIG*>(buffer.data());
	return config->dwStartType;
}

void ServiceManager::StartStop(const wstring& serviceName, bool start)
{
	TRACE(__FUNCTION__);

	TRACEP(L"Starting service: ", serviceName.c_str());

	Utils::AutoCloseServiceHandle serviceManagerHandle = OpenSCManager(NULL /*local machine*/, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
	if (serviceManagerHandle.Get() == NULL)
	{
		throw DMBridgeExceptionWithErrorCode("OpenSCManager() failed.", GetLastError());
	}

	Utils::AutoCloseServiceHandle serviceHandle = OpenService(serviceManagerHandle.Get() /*scm manager*/, serviceName.c_str(), SERVICE_ALL_ACCESS);
	if (serviceHandle.Get() == NULL)
	{
		throw DMBridgeExceptionWithErrorCode("OpenService() failed.", GetLastError());
	}

	SERVICE_STATUS serviceStatus;
	if (!QueryServiceStatus(serviceHandle.Get() /*service handle*/, &serviceStatus))
	{
		throw DMBridgeExceptionWithErrorCode("QueryServiceStatus() failed.", GetLastError());
	}

	if (start)
	{
		// Start...
		if (serviceStatus.dwCurrentState == SERVICE_RUNNING)
		{
			TRACE(L"Service is already running!");
			return;
		}

		if (!StartService(serviceHandle.Get() /*service handle*/, 0 /* arg count*/, NULL /* no args*/))
		{
			throw DMBridgeExceptionWithErrorCode("StartService() failed.", GetLastError());
		}
		TRACE(L"Service has been started successfully");
	}
	else
	{
		// Stop...
		if (serviceStatus.dwCurrentState != SERVICE_RUNNING)
		{
			TRACE(L"Service is already stopped!");
			return;
		}

		if (!ControlService(serviceHandle.Get() /*service handle*/, SERVICE_CONTROL_STOP, &serviceStatus))
		{
			throw DMBridgeExceptionWithErrorCode("ControlService() failed.", GetLastError());
		}
		TRACE(L"Service has been stopped successfully");
	}
}

void ServiceManager::Start(const wstring& serviceName)
{
	StartStop(serviceName, true /*start*/);
}

void ServiceManager::Stop(const wstring& serviceName)
{
	StartStop(serviceName, false /*stop*/);
}

void ServiceManager::SetStartType(const wstring& serviceName, DWORD startType)
{
	TRACE(__FUNCTION__);

	TRACEP(L"Enabling auto startup for service: ", serviceName.c_str());

	Utils::AutoCloseServiceHandle serviceManagerHandle = OpenSCManager(NULL /*local machine*/, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
	if (serviceManagerHandle.Get() == NULL)
	{
		throw DMBridgeExceptionWithErrorCode("OpenSCManager() failed.", GetLastError());
	}

	Utils::AutoCloseServiceHandle serviceHandle = OpenService(serviceManagerHandle.Get() /*scm manager*/, serviceName.c_str(), SERVICE_ALL_ACCESS);
	if (serviceHandle.Get() == NULL)
	{
		throw DMBridgeExceptionWithErrorCode("OpenService() failed.", GetLastError());
	}

	if (!ChangeServiceConfig(serviceHandle.Get(),
		SERVICE_NO_CHANGE,
		startType,
		SERVICE_NO_CHANGE,
		NULL, /*path not changing*/
		NULL, /*load order group not changing*/
		NULL, /*TagIId not changing*/
		NULL, /*dependencies not changing*/
		NULL, /*account nmae not changing*/
		NULL, /*password not changing*/
		NULL)) /*display name not changing*/
	{
		throw DMBridgeExceptionWithErrorCode("ChangeServiceConfig() to change service to auto start.", GetLastError());
	}
}
