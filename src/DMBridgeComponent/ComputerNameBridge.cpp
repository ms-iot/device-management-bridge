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
#include "ComputerNameBridge.h"
#include "RpcUtilities.h"

using namespace winrt;
using namespace RpcUtils;

namespace winrt::DMBridgeComponent::implementation
{
	/// <summary>
	/// Set the local computer name.
	/// </summary>
	/// <remarks>
	/// This function updates both the DNS Hostname and NetBIOS.
	/// <remarks>
	/// <param name='computerName'>The new computer name. Has a max length of 15 characters, and can only include alphanumeric characters.</param>
	/// <exception cref="ERROR_INVALID_COMPUTERNAME">Thrown if computerName is an invalid length or has illegal characters.</exception>
	void ComputerNameBridge::SetName(hstring computerName)
	{
		const wchar_t* cName = const_cast<wchar_t*>(computerName.c_str());
		check_hresult(
			RpcNormalize(::SetComputerNameRpc, this->rpcBinding, cName));
	}

	/// <remarks>
	/// Retrieve the NetBIOS name of the computer.
	/// <remarks>
	/// <returns>The local computer name.</returns>
	hstring ComputerNameBridge::GetName()
	{
		wchar_t* hostnamePtr = NULL;
		long size = 0;

		check_hresult(
			RpcNormalize(::GetComputerNameRpc, this->rpcBinding, &size, &hostnamePtr));

		// Copy the string before it is freed
		hstring hostname = hostnamePtr;
		MIDL_user_free(hostnamePtr);

		return hostname;
	}

	/// <remarks>
	/// If there is a rename pending, it will be applied on the next reboot.
	/// <remarks>
	/// <returns>True if a computer rename is pending</returns>
	bool ComputerNameBridge::IsRenamePending()
	{
		BOOL isPending = 0;
		check_hresult(
			RpcNormalize(::IsComputerRenamePendingRpc, this->rpcBinding, &isPending));
		return isPending;
	}
}
