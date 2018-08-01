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
#include "NTServiceBridge.h"
#include "RpcUtilities.h"

using namespace winrt;
using namespace RpcUtils;

namespace winrt::DMBridgeComponent::implementation
{
	/// <summary>
	/// Start the specified service. Only services present on a pre-defined whitelist can be manually started.
	/// </summary>
	/// <param name='serviceName'>The service name to start, it is case insensitive. Has a max length of 256 characters, and cannot include the forward or backward slash.</param>
	/// <exception cref="ERROR_ACCESS_DENIED">Thrown if the requested service is not whitelisted.</exception>
	/// <exception cref="ERROR_INVALID_SERVICENAME">Thrown if serviceName is an invalid length or has illegal characters.</exception>
	void NTServiceBridge::Start(hstring serviceName)
	{
		wchar_t* cName = const_cast<wchar_t*>(serviceName.c_str());
		check_hresult(
			RpcNormalize(::StartServiceRpc, this->rpcBinding, cName));
	}

	/// <summary>
	/// Stop the specified service. Only services present on a pre-defined whitelist can be manually stopped.
	/// </summary>
	/// <param name='serviceName'>The service name to stop, it is case insensitive. Has a max length of 256 characters, and cannot include the forward or backward slash.</param>
	/// <exception cref="ERROR_ACCESS_DENIED">Thrown if the requested service is not whitelisted.</exception>
	/// <exception cref="ERROR_INVALID_SERVICENAME">Thrown if serviceName is an invalid length or has illegal characters.</exception>
	void NTServiceBridge::Stop(hstring serviceName)
	{
		wchar_t* cName = const_cast<wchar_t*>(serviceName.c_str());
		check_hresult(
			RpcNormalize(::StopServiceRpc, this->rpcBinding, cName));
	}

	/// <summary>
	/// Query the state of a specific service. Non-whitelisted services can be queried.
	/// </summary>
	/// <param name='serviceName'>The service name to query, it is case insensitive. Has a max length of 256 characters, and cannot include the forward or backward slash.</param>
	/// <return>An integer representing the service state. See https://docs.microsoft.com/en-us/windows/desktop/api/winsvc/ns-winsvc-_service_status for the possible values.</return>
	/// <exception cref="ERROR_INVALID_SERVICENAME">Thrown if serviceName is an invalid length or has illegal characters.</exception>
	int32_t NTServiceBridge::Query(hstring serviceName)
	{
		wchar_t* cName = const_cast<wchar_t*>(serviceName.c_str());

		int32_t status = 0;

		check_hresult(
			RpcNormalize(::QueryServiceRpc, this->rpcBinding, cName, &status));

		return status;
	}
}
