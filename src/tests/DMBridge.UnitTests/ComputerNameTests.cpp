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
#include "RegistryUtils.h"

constexpr wchar_t* TcpNameKey = L"system\\currentcontrolset\\services\\tcpip\\parameters";
constexpr wchar_t* TcpNameValue = L"NV HostName";
constexpr wchar_t* CompNameKey1 = L"system\\currentcontrolset\\control\\computername\\computername";
constexpr wchar_t* CompNameKey2 = L"system\\currentcontrolset\\control\\computername\\activecomputername";
constexpr wchar_t* CompNameValue = L"ComputerName";

constexpr wchar_t* ALL_DIGITS_NAME = L"1234";
constexpr wchar_t* WHITESPACE_NAME = L"foo bar";
constexpr wchar_t* BACKSLASH_NAME = L"\\dhcp";
constexpr wchar_t* FORWARDSLASH_NAME = L"/dhcp";
constexpr wchar_t* ASCII_CONTROL_NAME = L"d\031hcp";
constexpr wchar_t* TOO_SHORT_NAME = L"";
constexpr wchar_t* TOO_LONG_NAME = L"AAAAAAAAAAAAAAAA";


// Provide two different options for a new computer name,
//   incase the computer's name already equals one of them
constexpr wchar_t* NewCompName = L"settest-1";
constexpr wchar_t* NewCompNameAlt = L"settest-2";

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace DMBridgeUnitTests
{
	TEST_CLASS(ComputerNameTests)
	{
	private:
		handle_t hRpcBinding = nullptr;
		std::wstring GetNetBIOSName()
		{
			wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
			DWORD dwSize = sizeof(buffer) / sizeof(wchar_t);
			GetComputerNameEx(ComputerNamePhysicalNetBIOS, buffer, &dwSize);
			std::wstring compName(buffer);

			return compName;
		}
		/* 
		Windows IoT's SetComputerName does not update all values
		so manually update the registry
		*/
		void SetComputerNameInRegistry(std::wstring name)
		{
			Utils::WriteRegistryValue(TcpNameKey, CompNameValue, name);
			Utils::WriteRegistryValue(CompNameKey1, CompNameValue, name);
			Utils::WriteRegistryValue(CompNameKey2, CompNameValue, name);
		}

		HRESULT GetComputerNameRpcFree(std::wstring& name)
		{
			wchar_t* computerNamePtr = NULL;
			long size = 0;

			HRESULT ret = ::GetComputerNameRpc(hRpcBinding, &size, &computerNamePtr);

			std::wstring actualComputerName = computerNamePtr;
			name = actualComputerName.c_str();

			MIDL_user_free(computerNamePtr);
			return ret;
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
		TEST_METHOD(GetName_NetBIOS_AreEqual)
		{
			// Arrange
			std::wstring expectedComputerName = GetNetBIOSName();
			std::wstring actualComputerName;

			// Act
			HRESULT ret = GetComputerNameRpcFree(actualComputerName);

			// Assert
			Assert::AreEqual(S_OK, ret);
			Assert::AreEqual(expectedComputerName.c_str(), actualComputerName.c_str());
		}

		TEST_METHOD(SetName_CurrentName_NoException)
		{
			// Arrange
			std::wstring currentName = GetNetBIOSName();

			// Act
			HRESULT ret = ::SetComputerNameRpc(hRpcBinding, currentName.c_str());

			// Assert
			Assert::AreEqual(S_OK, ret);
		}

		TEST_METHOD(SetName_UpdatesRegistry)
		{
			// Arrange
			std::wstring currentName = GetNetBIOSName();
			// Build a new name
			std::wstring newName = (_wcsicmp(currentName.c_str(), NewCompName) != 0)
				? NewCompName
				: NewCompNameAlt;

			// Act
			HRESULT ret = ::SetComputerNameRpc(hRpcBinding, newName.c_str());
			
			// Assert
			std::wstring tcpKey = Utils::ReadRegistryValue(TcpNameKey, TcpNameValue);
			std::wstring compKey1 = Utils::ReadRegistryValue(CompNameKey1, CompNameValue);
			std::wstring compKey2 = Utils::ReadRegistryValue(CompNameKey2, CompNameValue);

			// Undo the name change
			SetComputerNameInRegistry(currentName);

			Assert::AreEqual(S_OK, ret);
			Assert::AreEqual(newName.c_str(), tcpKey.c_str());
			Assert::AreEqual(newName.c_str(), compKey1.c_str());
			Assert::AreEqual(newName.c_str(), compKey2.c_str());
		}
		
		TEST_METHOD(IsPending_Pending_EqualsTrue)
		{
			// Arrange
			BOOL isPending = false;
			std::wstring currentName = GetNetBIOSName();
			// Build a new name
			std::wstring newName = (_wcsicmp(currentName.c_str(), NewCompName) != 0)
				? NewCompName
				: NewCompNameAlt;

			// Ensure there is a pending rename
			SetComputerNameInRegistry(newName);

			// Act
			HRESULT ret = ::IsComputerRenamePendingRpc(hRpcBinding, &isPending);
			
			// Undo computer name change for test
			SetComputerNameInRegistry(currentName);

			// Assert
			Assert::AreEqual(S_OK, ret);
			Assert::IsTrue(isPending);
		}

		TEST_METHOD(IsPending_NonePending_EqualsFalse)
		{
			// Arrange
			BOOL isPending = false;
			std::wstring currentName = GetNetBIOSName();

			// Undo any pending renames
			SetComputerNameInRegistry(currentName);

			// Act
			HRESULT ret = ::IsComputerRenamePendingRpc(hRpcBinding, &isPending);

			// Assert
			Assert::AreEqual(S_OK, ret);
			Assert::IsFalse(isPending);
		}

		/* INPUT VALIDATION */
		/* ====================================== */

		TEST_METHOD(SetName_TooLong_InvalidComputerName)
		{
			HRESULT ret = ::SetComputerNameRpc(hRpcBinding, TOO_LONG_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_COMPUTERNAME), ret);
		}

		TEST_METHOD(SetName_TooShort_InvalidComputerName)
		{
			HRESULT ret = ::SetComputerNameRpc(hRpcBinding, TOO_SHORT_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_COMPUTERNAME), ret);
		}

		TEST_METHOD(SetName_ASCIIControl_InvalidComputerName)
		{
			HRESULT ret = ::SetComputerNameRpc(hRpcBinding, ASCII_CONTROL_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_COMPUTERNAME), ret);
		}

		TEST_METHOD(SetName_ForwardSlash_InvalidComputerName)
		{
			HRESULT ret = ::SetComputerNameRpc(hRpcBinding, FORWARDSLASH_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_COMPUTERNAME), ret);
		}

		TEST_METHOD(SetName_BackSlash_InvalidComputerName)
		{
			HRESULT ret = ::SetComputerNameRpc(hRpcBinding, BACKSLASH_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_COMPUTERNAME), ret);
		}

		TEST_METHOD(SetName_WhiteSpace_InvalidComputerName)
		{
			HRESULT ret = ::SetComputerNameRpc(hRpcBinding, WHITESPACE_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_COMPUTERNAME), ret);
		}

		TEST_METHOD(SetName_AllDigits_InvalidComputerName)
		{
			HRESULT ret = ::SetComputerNameRpc(hRpcBinding, ALL_DIGITS_NAME);
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_INVALID_COMPUTERNAME), ret);
		}
	};
}