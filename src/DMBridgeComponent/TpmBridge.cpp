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
#include "TpmBridge.h"
#include "RpcUtilities.h"

using namespace winrt;
using namespace RpcUtils;

namespace winrt::DMBridgeComponent::implementation
{
    /// <remarks>
    /// Retrieve the endorsement key from the TPM.
    /// <remarks>
    /// <returns>The endorsement key.</returns>
    winrt::hstring TpmBridge::GetEndorsementKey()
    {
        wchar_t* ekPtr = NULL;
        int size = 0;

        check_hresult(
            RpcNormalize(::GetEndorsementKeyRpc, this->rpcBinding, &size, &ekPtr));

        // Copy the string before it is freed
        hstring ek = ekPtr;
        MIDL_user_free(ekPtr);

        return ek;
    }

    /// <remarks>
    /// Retrieve the registration id from the TPM.
    /// <remarks>
    /// <returns>The registration id.</returns>
    hstring TpmBridge::GetRegistrationId()
    {
        wchar_t* regIdPtr = NULL;
        int size = 0;

        check_hresult(
            RpcNormalize(::GetRegistrationIdRpc, this->rpcBinding, &size, &regIdPtr));

        // Copy the string before it is freed
        hstring regId = regIdPtr;
        MIDL_user_free(regIdPtr);

        return regId;
    }

    /// <remarks>
    /// Retrieve the connection string from the TPM.
    /// <remarks>
    /// <returns>The connection string.</returns>
    hstring TpmBridge::GetConnectionString(INT32 slot, INT32 expiryInSeconds)
    {
        wchar_t* csPtr = NULL;
        int size = 0;

        check_hresult(
            RpcNormalize(::GetConnectionStringRpc, this->rpcBinding, slot, expiryInSeconds, &size, &csPtr));

        // Copy the string before it is freed
        hstring cs = csPtr;
        MIDL_user_free(csPtr);

        return cs;
    }
}
