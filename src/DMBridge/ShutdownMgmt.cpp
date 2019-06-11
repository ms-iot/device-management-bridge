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
#include "ShutdownMgmt.h"
#include "Logger.h"
#include "DMProcess.h"

using namespace std;

constexpr wchar_t ShutdownExe[] = L"%windir%\\system32\\shutdown.exe";
constexpr wchar_t ShutdownExeDelay[] = L" -t ";
constexpr wchar_t ShutdownExeRestart[] = L" -r ";

/* Map Generated rpc method signatures to class */
/* -------------------------------------------- */

HRESULT ShutdownRpc(_In_ handle_t, _In_ INT32 delayInSeconds, _In_ boolean restart)
{
    return ShutdownMgmt::Shutdown(delayInSeconds, restart);
}

/* -------------------------------------------- */

HRESULT ShutdownExePath(wstring& path)
{
    wchar_t expanded[MAX_PATH] = { 0 };
    size_t length = ExpandEnvironmentStrings(ShutdownExe, expanded, MAX_PATH);
    if (length == 0 || length > MAX_PATH)
    {
        return E_FAIL;
    }

    path = expanded;

    return S_OK;
}

HRESULT ShutdownMgmt::Shutdown(_In_ INT32 delayInSeconds, _In_ boolean restart)
{
    TRACE(__FUNCTION__);

    wstring shutdownExePath;
    HRESULT hr = ShutdownExePath(shutdownExePath);
    if (FAILED(hr))
    {
        return hr;
    }

    wstring fullCommand;
    fullCommand += shutdownExePath;
    fullCommand += ShutdownExeDelay;
    fullCommand += to_wstring(delayInSeconds);
    if (restart)
    {
        fullCommand += ShutdownExeRestart;
    }

    unsigned long returnCode = 0;
    string output;
    Process::Launch(fullCommand, returnCode, output);

    return returnCode == 0 ? S_OK : E_FAIL;
}
