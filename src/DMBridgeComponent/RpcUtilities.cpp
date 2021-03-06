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
#include "RpcUtilities.h"
#include "RpcConstants.h"

namespace RpcUtils
{
	RPC_STATUS RpcBind(RPC_BINDING_HANDLE* rpcHandle)
	{
		RPC_STATUS status = RPC_S_OK;
		RPC_WSTR stringBinding = nullptr;

		status = RpcStringBindingCompose(
			NULL,
			(RPC_WSTR)RPC_PROTOCOL_SEQUENCE,
			NULL,
			(RPC_WSTR)RPC_ENDPOINT,
			NULL,
			&stringBinding);
		if (status != RPC_S_OK)
		{
			return status;
		}

		status = RpcBindingFromStringBinding(stringBinding, rpcHandle);
		if (status != RPC_S_OK)
		{
			return status;
		}

		RpcStringFree(&stringBinding);
		return status;
	}

	void RpcCloseBinding(RPC_BINDING_HANDLE* rpcHandle)
	{
		if (rpcHandle == nullptr)
			return;

		RpcBindingFree(rpcHandle);
		rpcHandle = nullptr;
	}
}

/******************************************************/
/*         MIDL allocate and free                     */
/******************************************************/

void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
	return(malloc(len));
}

void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
	free(ptr);
}