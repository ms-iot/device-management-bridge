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

#include "pch.h"
#include "TelemetryLevelBridge.h"
#include "RpcUtilities.h"

using namespace winrt;
using namespace RpcUtils;

namespace winrt::DMBridgeComponent::implementation
{
	/// <summary>
	/// Set the telemetry level.
	/// </summary>
	/// <param name='level'>The desired telemetry level
	/// 0 = Security
	/// 1 = Basic
	/// 2 = Enhanced
	/// 3 = Full
	/// </param>
	void TelemetryLevelBridge::SetLevel(int32_t level)
	{
		check_hresult(
			RpcNormalize(::SetTelemetryLevelRpc, this->rpcBinding, level));
	}

	/// <summary>
	/// Get the telemetry level.
	/// </summary>
	/// <return>An integer representing the telemetry level.
	/// 0 = Security
	/// 1 = Basic
	/// 2 = Enhanced
	/// 3 = Full
	/// </return>
	int32_t TelemetryLevelBridge::GetLevel()
	{
		int32_t level = 0;
		check_hresult(
			RpcNormalize(::GetTelemetryLevelRpc, this->rpcBinding, &level));

		return level;
	}
}
