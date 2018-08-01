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
#include <filesystem>
#include <assert.h>
#include "DMBridgeService.h"
#include "Logger.h"
#include "DMBridgeException.h"

using namespace std;
using namespace std::chrono;
using namespace std::experimental;

DMBridgeService *DMBridgeService::s_service = NULL;

void DMBridgeService::Run(DMBridgeService &service)
{
	TRACE(__FUNCTION__);

	s_service = &service;

	SERVICE_TABLE_ENTRY serviceTable[] =
	{
		{ const_cast<LPWSTR>(service._name.c_str()), ServiceMain },
		{ NULL, NULL }
	};

	if (!StartServiceCtrlDispatcher(serviceTable))
	{
		throw DMBridgeExceptionWithErrorCode(GetLastError());
	}
}

void WINAPI DMBridgeService::ServiceMain(DWORD, PWSTR*)
{
	TRACE(__FUNCTION__);
	assert(s_service != NULL);

	s_service->_statusHandle = RegisterServiceCtrlHandler(s_service->_name.c_str(), ServiceCtrlHandler);
	if (s_service->_statusHandle == NULL)
	{
		throw DMBridgeExceptionWithErrorCode(GetLastError());
	}

	s_service->Start();
}

void WINAPI DMBridgeService::ServiceCtrlHandler(DWORD ctrl)
{
	TRACE(__FUNCTION__);

	switch (ctrl)
	{
	case SERVICE_CONTROL_STOP:
		TRACE("Service stop received...");
		s_service->Stop();
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		TRACE("Service shutdown received...");
		s_service->Shutdown();
		break;
	case SERVICE_CONTROL_INTERROGATE:
		TRACE("Service interrogate received...");
		break;
	default: break;
	}
}

DMBridgeService::DMBridgeService(const wstring& serviceName)
{
	TRACE(__FUNCTION__);
	assert(serviceName.size() != 0);

	_name = serviceName;
	_statusHandle = NULL;

	_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	_status.dwCurrentState = SERVICE_START_PENDING;

	DWORD dwControlsAccepted = 0;
	dwControlsAccepted |= SERVICE_ACCEPT_STOP;
	dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
	_status.dwControlsAccepted = dwControlsAccepted;

	_status.dwWin32ExitCode = NO_ERROR;
	_status.dwServiceSpecificExitCode = 0;
	_status.dwCheckPoint = 0;
	_status.dwWaitHint = 0;
}

void DMBridgeService::Start()
{
	TRACE(__FUNCTION__);
	try
	{
		SetServiceStatus(SERVICE_START_PENDING);
		OnStart();
		SetServiceStatus(SERVICE_RUNNING);
	}
	catch (const DMBridgeExceptionWithErrorCode&)
	{
		WriteEventLogEntry(L"Service failed to start.", EVENTLOG_ERROR_TYPE);
		SetServiceStatus(SERVICE_STOPPED);
	}
}

void DMBridgeService::Stop()
{
	TRACE(__FUNCTION__);

	DWORD originalState = _status.dwCurrentState;
	try
	{
		SetServiceStatus(SERVICE_STOP_PENDING);
		OnStop();
		SetServiceStatus(SERVICE_STOPPED);
	}
	catch (const DMBridgeExceptionWithErrorCode&)
	{
		WriteEventLogEntry(L"Service failed to stop.", EVENTLOG_ERROR_TYPE);
		SetServiceStatus(originalState);
	}
}

void DMBridgeService::Shutdown()
{
	TRACE(__FUNCTION__);
}

void DMBridgeService::SetServiceStatus(DWORD currentState, DWORD win32ExitCode)
{
	static DWORD checkPoint = 1;

	_status.dwCurrentState = currentState;
	_status.dwWin32ExitCode = win32ExitCode;
	_status.dwWaitHint = 0;
	_status.dwCheckPoint = ((currentState == SERVICE_RUNNING) || (currentState == SERVICE_STOPPED)) ? 0 : checkPoint++;

	if (!::SetServiceStatus(_statusHandle, &_status))
	{
		WriteErrorLogEntry(L"Error: Could not set service status. Error Code ", GetLastError());
	}
}

void DMBridgeService::WriteEventLogEntry(const wstring& message, WORD type)
{
	TRACE(message.c_str());

	HANDLE eventSource = RegisterEventSource(NULL, _name.c_str());
	if (!eventSource)
	{
		TRACEP(L"Error: Failed to register the event source. Error Code ", GetLastError());
		// No exception is thrown since it is not a scenario that should affect normal operations.
		return;
	}

	LPCWSTR strings[2] = {
		_name.c_str(),
		message.c_str()
	};

	if (!ReportEvent(eventSource,   // Event log handle
		type,                  // Event type
		0,                     // Event category
		0,                     // Event identifier
		NULL,                  // No security identifier
		2,                     // Size of strings array
		0,                     // No binary data
		strings,               // Array of strings
		NULL                   // No binary data
	))
	{
		TRACEP(L"Error: Failed to write to the event log. Error Code ", GetLastError());
		// No exception is thrown since it is not a scenario that should affect normal operations.
	}

	if (!DeregisterEventSource(eventSource))
	{
		TRACEP(L"Error: Failed to unregister the event source. Error Code ", GetLastError());
		// No exception is thrown since it is not a scenario that should affect normal operations.
	}
}

void DMBridgeService::WriteErrorLogEntry(const wstring& function, DWORD errorCode)
{
	basic_ostringstream<wchar_t> message;
	message << function << L" failed w/err " << errorCode;
	WriteEventLogEntry(message.str(), EVENTLOG_ERROR_TYPE);
}

void DMBridgeService::OnStart()
{
	TRACE(__FUNCTION__);

	_workerThread = thread(ServiceWorkerThread, this);
}

void DMBridgeService::ServiceWorkerThread(void* context)
{
	TRACE(__FUNCTION__);
	DMBridgeServer::Setup();
	DMBridgeService* iotDMBridgeService = static_cast<DMBridgeService*>(context);
	iotDMBridgeService->ServiceWorkerThreadHelper();
}

void DMBridgeService::ServiceWorkerThreadHelper(void)
{
	TRACE(__FUNCTION__);
	DMBridgeServer::Listen();
	TRACE("Worker thread exiting.");
}

void DMBridgeService::OnStop()
{
	TRACE(__FUNCTION__);
	DMBridgeServer::StopListening();
}

void DMBridgeService::Install(
	const wstring& serviceName,
	const wstring& displayName,
	DWORD startType,
	const wstring& dependencies,
	const wstring& account,
	const wstring& password)
{
	TRACE(__FUNCTION__);

	wchar_t szPath[MAX_PATH];
	if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)) == 0)
	{
		TRACEP(L"GetModuleFileName failed w/err :", GetLastError());
		return;
	}

	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
	if (schSCManager == NULL)
	{
		TRACEP(L"OpenSCManager failed w/err :", GetLastError());
		return;
	}

	// Install the service into SCM by calling CreateService
	SC_HANDLE schService = CreateService(
		schSCManager,                   // SCManager database
		serviceName.c_str(),            // Name of service
		displayName.c_str(),            // Name to display
		SERVICE_QUERY_STATUS,           // Desired access
		SERVICE_WIN32_OWN_PROCESS,      // Service type
		startType,                      // Service start type
		SERVICE_ERROR_NORMAL,           // Error control type
		szPath,                         // Service's binary
		NULL,                           // No load ordering group
		NULL,                           // No tag identifier
		dependencies.c_str(),           // Dependencies
		account.c_str(),                // Service running account
		password.c_str()                // Password of the account
	);

	if (schService != NULL)
	{
		CloseServiceHandle(schService);
		schService = NULL;
		TRACE(L"Installed successfully!");
	}
	else
	{
		TRACEP(L"CreateService failed w/err :", GetLastError());
	}

	CloseServiceHandle(schSCManager);
	schSCManager = NULL;
}

void DMBridgeService::Uninstall(const wstring& serviceName)
{
	TRACE(__FUNCTION__);

	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL)
	{
		TRACEP(L"OpenSCManager failed w/err :", GetLastError());
		return;
	}

	// Open the service with delete, stop, and query status permissions
	SC_HANDLE schService = OpenService(schSCManager, serviceName.c_str(), SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
	if (schService != NULL)
	{
		// Try to stop the service
		SERVICE_STATUS ssSvcStatus = {};
		if (ControlService(schService, SERVICE_CONTROL_STOP, &ssSvcStatus))
		{
			TRACEP(L"Stopping ", serviceName);
			Sleep(1000);

			while (QueryServiceStatus(schService, &ssSvcStatus))
			{
				if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING)
				{
					TRACE(L".");
					Sleep(1000);
				}
				else
				{
					break;
				}
			}

			if (ssSvcStatus.dwCurrentState == SERVICE_STOPPED)
			{
				TRACE(L"\nStopped.");
			}
			else
			{
				TRACE(L"\nError: Failed to stop.");
			}
		}

		// Now remove the service by calling DeleteService.
		if (DeleteService(schService))
		{
			TRACE(L"Service uninstalled.");
		}
		else
		{
			TRACEP(L"DeleteService failed w/err :", GetLastError());
		}

		CloseServiceHandle(schService);
		schService = NULL;
	}
	else
	{
		TRACEP(L"OpenService failed w/err :", GetLastError());
	}

	CloseServiceHandle(schSCManager);
	schSCManager = NULL;
}
