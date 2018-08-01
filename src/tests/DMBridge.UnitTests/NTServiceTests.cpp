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
#include "CppUnitTest.h"
#include "DMBridgeUnitTests.h"
#include <mutex>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

constexpr wchar_t* NON_WHITELISTED_SERVICE = L"dhcp";
constexpr wchar_t* WHITELISTED_SERVICE = L"w32time";
constexpr wchar_t* WHITELISTED_ALTERNATIVECASE_SERVICE = L"W32TIme";
constexpr wchar_t* BACKSLASH_NAME = L"\\dhcp";
constexpr wchar_t* FORWARDSLASH_NAME = L"/dhcp";
constexpr wchar_t* ASCII_CONTROL_NAME = L"d\031hcp";
constexpr wchar_t* TOO_SHORT_NAME = L"";
constexpr wchar_t* TOO_LONG_NAME = L"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

using namespace std;

constexpr unsigned int MAX_SERVICE_WAIT_SECONDS = 10;

namespace DMBridgeUnitTests
{
	TEST_CLASS(NTServiceTests)
	{
	private:
		handle_t hRpcBinding = nullptr;

		SC_HANDLE GetService(const wchar_t *serviceName, DWORD serviceControlManagerPermissions, DWORD servicePermission)
		{
			SC_HANDLE service = NULL;
			SC_HANDLE serviceControlManager = OpenSCManager(NULL, NULL, serviceControlManagerPermissions);
			if (serviceControlManager == NULL)
			{
				throw GetLastError();
			}

			service = OpenService(serviceControlManager, serviceName, servicePermission);
			CloseServiceHandle(serviceControlManager);
			return service;
		}
		DWORD GetServiceStatus(const wchar_t *serviceName)
		{
			SC_HANDLE service = GetService(serviceName, 0, SERVICE_QUERY_STATUS);
			SERVICE_STATUS_PROCESS stat;
			DWORD needed = 0;
			bool success = QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (BYTE*)&stat, sizeof stat, &needed);
			CloseServiceHandle(service);
			if (!success)
			{
				throw GetLastError();
			}
			return stat.dwCurrentState;
		}
		void RunService(const wchar_t *serviceName, bool wait = false)
		{
			DWORD status = GetServiceStatus(serviceName);
			if (status == SERVICE_RUNNING || status == SERVICE_START_PENDING)
				return;

			SC_HANDLE service = GetService(serviceName, 0, SERVICE_START);
			bool success = StartService(service, 0, NULL);
			CloseServiceHandle(service);

			if (wait)
			{
				WaitForStatus(serviceName, SERVICE_RUNNING, MAX_SERVICE_WAIT_SECONDS);
			}
		}
		void StopService(const wchar_t *serviceName, bool wait = false)
		{
			DWORD status = GetServiceStatus(serviceName);
			if (status == SERVICE_STOPPED || status == SERVICE_STOP_PENDING)
			{
				return;
			}

			SC_HANDLE service = GetService(serviceName, 0, SERVICE_STOP);

			SERVICE_STATUS serviceStatus;
			bool success = ControlService(service, SERVICE_CONTROL_STOP, &serviceStatus);
			CloseServiceHandle(service);

			if (wait)
			{
				WaitForStatus(serviceName, SERVICE_STOPPED, MAX_SERVICE_WAIT_SECONDS);
			}
		}
		bool WaitForStatus(const wchar_t *serviceName, DWORD status, unsigned int maxWaitInSeconds)
		{
			while (maxWaitInSeconds--)
			{
				if (status == GetServiceStatus(serviceName))
				{
					return true;
				}
				::Sleep(1000);
				--maxWaitInSeconds;
			}
			return false;
		}
	public:
		TEST_METHOD_INITIALIZE(Setup)
		{
			RpcSetup(&hRpcBinding);
		}
		TEST_METHOD_CLEANUP(TearDown)
		{
			RpcTearDown(&hRpcBinding);
		}

		/* FUNCTIONALITY */
		/* ====================================== */
		TEST_METHOD(Start_StoppedService_CorrectStatus)
		{
			// Arrange
			int32_t status = 0;
			int32_t expected = SERVICE_RUNNING;
			StopService(WHITELISTED_SERVICE, true);

			// Act
			HRESULT ret = ::StartServiceRpc(hRpcBinding, WHITELISTED_SERVICE);

			// Assert
			::QueryServiceRpc(hRpcBinding, WHITELISTED_SERVICE, &status);
			Assert::AreEqual(S_OK, ret);

			// The service may either be running or in the process of starting
			if (status != SERVICE_RUNNING)
				expected = SERVICE_START_PENDING;
			Assert::AreEqual(expected, status);
		}

		TEST_METHOD(Stop_StartedService_CorrectStatus)
		{
			// Arrange
			int32_t status = 0;
			int32_t expected = SERVICE_STOPPED;
			RunService(WHITELISTED_SERVICE, true);

			// Act
			HRESULT ret = ::StopServiceRpc(hRpcBinding, WHITELISTED_SERVICE);

			// Assert
			::QueryServiceRpc(hRpcBinding, WHITELISTED_SERVICE, &status);
			Assert::AreEqual(S_OK, ret);

			// The service may either be stopped or in the process of stopping
			if (status != SERVICE_STOPPED)
				expected = SERVICE_STOP_PENDING;
			Assert::AreEqual(expected, status);
		}

		TEST_METHOD(Start_AlreadyStarted_NoException)
		{
			// Arrange
			RunService(WHITELISTED_SERVICE, true);

			// Act
			HRESULT ret = ::StartServiceRpc(hRpcBinding, WHITELISTED_SERVICE);

			// Assert
			Assert::AreEqual(S_OK, ret);
		}

		TEST_METHOD(Stop_AlreadyStopped_NoException)
		{
			// Arrange
			StopService(WHITELISTED_SERVICE, true);

			// Act
			HRESULT ret = ::StopServiceRpc(hRpcBinding, WHITELISTED_SERVICE);

			// Assert
			// Start up the service since it was stopped in Arrange
			RunService(WHITELISTED_SERVICE, true);
			Assert::AreEqual(S_OK, ret);
		}

		TEST_METHOD(Query_Started_CorrectStatus)
		{
			// Arrange
			int32_t status = 0;
			int32_t expected = SERVICE_RUNNING;
			RunService(WHITELISTED_SERVICE, true);

			// Act
			HRESULT ret = ::QueryServiceRpc(hRpcBinding, WHITELISTED_SERVICE, &status);

			// Assert
			// The service may either be running or in the process of starting
			if (status != SERVICE_RUNNING)
				expected = SERVICE_START_PENDING;
			Assert::AreEqual(expected, status);
		}

		TEST_METHOD(Query_Stopped_CorrectStatus)
		{
			// Arrange
			int32_t status = 0;
			int32_t expected = SERVICE_STOPPED;
			StopService(WHITELISTED_SERVICE, true);

			// Act
			HRESULT ret = ::QueryServiceRpc(hRpcBinding, WHITELISTED_SERVICE, &status);

			// Assert
			// Start up the service since it was stopped in Arrange
			RunService(WHITELISTED_SERVICE, true);
			// The service may either be stopped or in the process of stopping
			if (status != SERVICE_STOPPED)
				expected = SERVICE_STOP_PENDING;
			Assert::AreEqual(expected, status);
		}

		/* INPUT VALIDATION */
		/* ====================================== */

		/* Whitelist */
		TEST_METHOD(Start_WhitelistedService_NoAccessDenied)
		{
			HRESULT ret = ::StartServiceRpc(hRpcBinding, WHITELISTED_SERVICE);
			Assert::AreNotEqual(HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), ret);
		}

		TEST_METHOD(Stop_WhitelistedService_NoAccessDenied)
		{
			HRESULT ret = ::StopServiceRpc(hRpcBinding, WHITELISTED_SERVICE);
			Assert::AreNotEqual(HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), ret);
		}

		TEST_METHOD(Query_WhitelistedServiceCasing_NoAccessDenied)
		{
			int32_t status = 0;
			HRESULT ret = ::QueryServiceRpc(hRpcBinding, WHITELISTED_SERVICE, &status);
			Assert::AreNotEqual(HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), ret);
		}

		TEST_METHOD(Start_WhitelistedServiceAltCasing_NoAccessDenied)
		{
			HRESULT ret = ::StartServiceRpc(hRpcBinding, WHITELISTED_ALTERNATIVECASE_SERVICE);
			Assert::AreNotEqual(HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), ret);
		}

		TEST_METHOD(Stop_WhitelistedServiceAltCasing_NoAccessDenied)
		{
			HRESULT ret = ::StopServiceRpc(hRpcBinding, WHITELISTED_ALTERNATIVECASE_SERVICE);
			Assert::AreNotEqual(HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), ret);
		}

		TEST_METHOD(Query_WhitelistedServiceAltCasing_NoAccessDenied)
		{
			int32_t status = 0;
			HRESULT ret = ::QueryServiceRpc(hRpcBinding, WHITELISTED_ALTERNATIVECASE_SERVICE, &status);
			Assert::AreNotEqual(HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), ret);
		}

		/* NonWhitelist */
		TEST_METHOD(Start_NonWhitelistedService_AccessDenied)
		{
			HRESULT ret = ::StartServiceRpc(hRpcBinding, NON_WHITELISTED_SERVICE);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), ret);
		}

		TEST_METHOD(Stop_NonWhitelistedService_AccessDenied)
		{
			HRESULT ret = ::StopServiceRpc(hRpcBinding, NON_WHITELISTED_SERVICE);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), ret);
		}

		TEST_METHOD(Query_NonWhitelistedService_NoAccessDenied)
		{
			int32_t status = 0;
			HRESULT ret = ::QueryServiceRpc(hRpcBinding, NON_WHITELISTED_SERVICE, &status);
			Assert::AreNotEqual(HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED), ret);
		}

		/* BackSlash */
		TEST_METHOD(Start_BackSlash_InvalidServiceName)
		{
			HRESULT ret = ::StartServiceRpc(hRpcBinding, BACKSLASH_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

		TEST_METHOD(Query_BackSlash_InvalidServiceName)
		{
			int32_t status = 0;
			HRESULT ret = ::QueryServiceRpc(hRpcBinding, BACKSLASH_NAME, &status);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

		TEST_METHOD(Stop_BackSlash_InvalidServiceName)
		{
			HRESULT ret = ::StopServiceRpc(hRpcBinding, BACKSLASH_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

		/* ForwardSlash */
		TEST_METHOD(Start_ForwardSlash_InvalidServiceName)
		{
			HRESULT ret = ::StartServiceRpc(hRpcBinding, FORWARDSLASH_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

		TEST_METHOD(Query_ForwardSlash_InvalidServiceName)
		{
			int32_t status = 0;
			HRESULT ret = ::QueryServiceRpc(hRpcBinding, FORWARDSLASH_NAME, &status);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

		TEST_METHOD(Stop_ForwardSlash_InvalidServiceName)
		{
			HRESULT ret = ::StopServiceRpc(hRpcBinding, FORWARDSLASH_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

		/* ASCIIControl */
		TEST_METHOD(Start_ASCIIControl_InvalidServiceName)
		{
			HRESULT ret = ::StartServiceRpc(hRpcBinding, ASCII_CONTROL_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

		TEST_METHOD(Query_ASCIIControl_InvalidServiceName)
		{
			int32_t status = 0;
			HRESULT ret = ::QueryServiceRpc(hRpcBinding, ASCII_CONTROL_NAME, &status);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

		TEST_METHOD(Stop_ASCIIControl_InvalidServiceName)
		{
			HRESULT ret = ::StopServiceRpc(hRpcBinding, ASCII_CONTROL_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

		/* Empty */
		TEST_METHOD(Start_Empty_InvalidServiceName)
		{
			HRESULT ret = ::StartServiceRpc(hRpcBinding, TOO_SHORT_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

		TEST_METHOD(Query_Empty_InvalidServiceName)
		{
			int32_t status = 0;
			HRESULT ret = ::QueryServiceRpc(hRpcBinding, TOO_SHORT_NAME, &status);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

		TEST_METHOD(Stop_Empty_InvalidServiceName)
		{
			HRESULT ret = ::StopServiceRpc(hRpcBinding, TOO_SHORT_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

		/* Max */
		TEST_METHOD(Start_TooLong_InvalidServiceName)
		{
			HRESULT ret = ::StartServiceRpc(hRpcBinding, TOO_LONG_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

		TEST_METHOD(Query_TooLong_InvalidServiceName)
		{
			int32_t status = 0;
			HRESULT ret = ::QueryServiceRpc(hRpcBinding, TOO_LONG_NAME, &status);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

		TEST_METHOD(Stop_TooLong_InvalidServiceName)
		{
			HRESULT ret = ::StopServiceRpc(hRpcBinding, TOO_LONG_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_SERVICENAME), ret);
		}

	};
}