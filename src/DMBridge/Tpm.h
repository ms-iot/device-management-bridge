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

#pragma once

#include "stdafx.h"

class Tpm
{
public:
    static HRESULT GetEndorsementKey(_Outptr_ int &size, _Outptr_ wchar_t *&ek);
    static HRESULT GetRegistrationId(_Outptr_ int &size, _Outptr_ wchar_t *&regId);
    static HRESULT GetConnectionString(_In_ int slot, _In_ int expiryInSeconds, _Outptr_ int &size, _Outptr_ wchar_t *&cs);
private:
    static HRESULT WriteRpcOutputString(const std::wstring& value, _Outptr_ int &rawValueSize, _Outptr_ wchar_t *&rawValue);
    static std::string Tpm::RunLimpet(const std::wstring& params);
    static HRESULT GetHostNameAndDeviceId(int logicalId, std::string& serviceUrl);
    static HRESULT GetSASToken(int logicalId, unsigned int durationInSeconds, std::string& sasToken);
};