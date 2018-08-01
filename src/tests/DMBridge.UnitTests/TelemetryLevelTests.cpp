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

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

constexpr wchar_t* TelemetryLevelKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\DataCollection";
constexpr wchar_t* TelemetryLevelValue = L"AllowTelemetry";
constexpr int MinTelemetryLevel = 0;
constexpr int MaxTelemetryLevel = 3;

namespace DMBridgeUnitTests
{
	TEST_CLASS(TelemetryLevelTests)
	{
	private:
		handle_t hRpcBinding = nullptr;
	public:
		TEST_METHOD_INITIALIZE(Setup)
		{
			RpcSetup(&hRpcBinding);
		}
		TEST_METHOD_CLEANUP(TearDown)
		{
			RpcTearDown(&hRpcBinding);
		}

		TEST_METHOD(GetLevel_EqualsRegistry)
		{
			// Arrange
			unsigned long uExpectedLevel;
			int32_t expectedLevel;
			int32_t actualLevel;
			Utils::TryReadRegistryValue(TelemetryLevelKey, TelemetryLevelValue, uExpectedLevel);
			expectedLevel = static_cast<int32_t>(uExpectedLevel);

			// Act
			HRESULT ret = ::GetTelemetryLevelRpc(hRpcBinding, &actualLevel);

			// Assert
			Assert::AreEqual(S_OK, ret);
			Assert::AreEqual(expectedLevel, actualLevel);
		}

		TEST_METHOD(SetLevel_TooLow_ThrowsException)
		{
			// Arrange
			int32_t targetLevel = MinTelemetryLevel - 1;

			// Act
			HRESULT ret = ::SetTelemetryLevelRpc(hRpcBinding, targetLevel);

			// Assert
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS), ret);
		}

		TEST_METHOD(SetLevel_TooHigh_ThrowsException)
		{
			// Arrange
			int32_t targetLevel = MaxTelemetryLevel + 1;

			// Act
			HRESULT ret = ::SetTelemetryLevelRpc(hRpcBinding, targetLevel);

			// Assert
			Assert::AreEqual(HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS), ret);
		}

		TEST_METHOD(SetLevel_Current_NoException)
		{
			// Arrange
			unsigned long uCurrentLevel;
			int32_t currentLevel;
			Utils::TryReadRegistryValue(TelemetryLevelKey, TelemetryLevelValue, uCurrentLevel);
			currentLevel = static_cast<int32_t>(uCurrentLevel);

			// Act
			HRESULT ret = ::SetTelemetryLevelRpc(hRpcBinding, currentLevel);

			// Assert
			Assert::AreEqual(S_OK, ret);
		}

		TEST_METHOD(SetLevel_DifferentValue_UpdatesRegistry)
		{
			// Arrange
			unsigned long uCurrentLevel;
			int32_t currentLevel;
			Utils::TryReadRegistryValue(TelemetryLevelKey, TelemetryLevelValue, uCurrentLevel);
			currentLevel = static_cast<int32_t>(uCurrentLevel);

			int32_t targetLevel = ((currentLevel + 1) % MaxTelemetryLevel);

			// Act
			HRESULT ret = ::SetTelemetryLevelRpc(hRpcBinding, targetLevel);

			// Assert
			Assert::AreEqual(S_OK, ret);

			Utils::TryReadRegistryValue(TelemetryLevelKey, TelemetryLevelValue, uCurrentLevel);
			currentLevel = static_cast<int32_t>(uCurrentLevel);
			Assert::AreEqual(targetLevel, currentLevel);
		}

	};
}