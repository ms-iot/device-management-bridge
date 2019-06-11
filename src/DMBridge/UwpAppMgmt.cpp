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
#include "UwpAppMgmt.h"
#include "Logger.h"
#include "DMProcess.h"

using namespace std;

constexpr wchar_t IotStartupExe[] = L"%windir%\\system32\\iotstartup.exe";

constexpr wchar_t AddCmd[] = L" add ";
constexpr wchar_t RemoveCmd[] = L" remove ";
constexpr wchar_t Headless[] = L" headless ";
constexpr wchar_t Headed[] = L" headed ";

enum StartupType
{
    eUndefined = 0x00000000,
    eNone = 0x00000001,
    eForeground = 0x00000002,
    eBackground = 0x00000003
};

/* Map Generated rpc method signatures to class */
/* -------------------------------------------- */

HRESULT SetAppStartupRpc(_In_ handle_t, _In_ const wchar_t *pkgFamilyName, _In_ INT32 startupType)
{
    return UwpAppMgmt::SetAppStartup(pkgFamilyName, startupType);
}

/* -------------------------------------------- */

HRESULT IotStartupExePath(wstring& path)
{
    wchar_t expanded[MAX_PATH] = { 0 };
    size_t length = ExpandEnvironmentStrings(IotStartupExe, expanded, MAX_PATH);
    if (length == 0 || length > MAX_PATH)
    {
        return E_FAIL;
    }

    path = expanded;

    return S_OK;
}

HRESULT UwpAppMgmt::SetAppStartup(_In_ const std::wstring& pkgFamilyName, _In_ INT32 startupType)
{
    TRACE(__FUNCTION__);

    wstring iotStartupExePath;
    HRESULT hr = IotStartupExePath(iotStartupExePath);
    if (FAILED(hr))
    {
        return hr;
    }

    wstring fullCommand;
    fullCommand += iotStartupExePath;

    switch (startupType)
    {
    case StartupType::eForeground:
        fullCommand += AddCmd;
        fullCommand += Headed;
        break;
    case StartupType::eBackground:
        fullCommand += AddCmd;
        fullCommand += Headless;
        break;
    case StartupType::eNone:
        // In case of none, we assume its a headless app. Removing headed app is not supported. 
        fullCommand += RemoveCmd;
        fullCommand += Headless;
        break;
    default:
        return E_FAIL;
    }

    fullCommand += pkgFamilyName;

    unsigned long returnCode = 0;
    string output;
    Process::Launch(fullCommand, returnCode, output);
    if (returnCode != 0)
    {
        if ((startupType == StartupType::eNone) && ((HRESULT)returnCode == E_INVALIDARG))
        {
            // OK to return if the app was not registered as startup and we are trying to remove it
            return S_OK;
        }
        return E_FAIL;
    }

    return S_OK;
}

