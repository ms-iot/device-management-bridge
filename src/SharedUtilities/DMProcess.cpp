/*
Copyright 2017 Microsoft
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
#include <windows.h>
#include <Psapi.h>
#include <vector>
#include <functional>
#include "DMProcess.h"
#include "AutoCloseHandle.h"
#include "Logger.h"
#include "DMBridgeException.h"

static const int ERROR_PIPE_HAS_BEEN_ENDED = 109;

using namespace std;
using namespace Utils;

void Process::Launch(
    const std::wstring& commandString,
    unsigned long& returnCode,
    std::string& output)
{
    TRACE(__FUNCTION__);

    SECURITY_ATTRIBUTES securityAttributes;
    securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAttributes.bInheritHandle = TRUE;
    securityAttributes.lpSecurityDescriptor = NULL;

    AutoCloseHandle stdOutReadHandle;
    AutoCloseHandle stdOutWriteHandle;
    DWORD pipeBufferSize = 4096;

    if (!CreatePipe(stdOutReadHandle.GetAddress(), stdOutWriteHandle.GetAddress(), &securityAttributes, pipeBufferSize))
    {
        throw DMBridgeExceptionWithErrorCode("Failed to create anonymous pipe for reading/writing child process stdin|stdout", GetLastError());
    }

    if (!SetHandleInformation(stdOutReadHandle.Get(), HANDLE_FLAG_INHERIT, 0 /*flags*/))
    {
        throw DMBridgeExceptionWithErrorCode("Failed to set handle information for stdout read handler", GetLastError());
    }

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = stdOutWriteHandle.Get();
    siStartInfo.hStdOutput = stdOutWriteHandle.Get();
    siStartInfo.hStdInput = NULL;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcess(NULL,
        const_cast<wchar_t*>(commandString.c_str()), // command line
        NULL,         // process security attributes
        NULL,         // primary thread security attributes
        TRUE,         // handles are inherited
        0,            // creation flags
        NULL,         // use parent's environment
        NULL,         // use parent's current directory
        &siStartInfo, // STARTUPINFO pointer
        &piProcInfo)) // receives PROCESS_INFORMATION
    {
        throw DMBridgeExceptionWithErrorCode("Failed to create process", GetLastError());
    }
    TRACE("Child process has been launched.");

    bool doneWriting = false;
    while (!doneWriting)
    {
        // Let the child process run for 1 second, and then check if there is anything to read...
        DWORD waitStatus = WaitForSingleObject(piProcInfo.hProcess, 1000);
        if (waitStatus == WAIT_OBJECT_0)
        {
            TRACE("Child process has exited.");
            if (!GetExitCodeProcess(piProcInfo.hProcess, &returnCode))
            {
                // We just ignore - something wrong with the API took place.
                TRACEP("Warning: Failed to get process exist code. GetLastError() = ", GetLastError());
            }
            CloseHandle(piProcInfo.hProcess);
            CloseHandle(piProcInfo.hThread);

            // Child process has exited, no more writing will take place.
            // Without closing the write channel, the ReadFile will keep waiting.
            doneWriting = true;
            stdOutWriteHandle.Close();
        }
        else
        {
            TRACE("Child process is still running...");
        }

        DWORD bytesAvailable = 0;
        if (PeekNamedPipe(stdOutReadHandle.Get(), NULL, 0, NULL, &bytesAvailable, NULL))
        {
            if (bytesAvailable > 0)
            {
                DWORD readByteCount = 0;
                vector<char> readBuffer(bytesAvailable + 1);
                if (ReadFile(stdOutReadHandle.Get(), readBuffer.data(), static_cast<unsigned int>(readBuffer.size()) - 1, &readByteCount, NULL) || readByteCount == 0)
                {
                    readBuffer[readByteCount] = '\0';
                    output += readBuffer.data();
                }
            }
        }
        else
        {
            DWORD retCode = GetLastError();
            if (ERROR_PIPE_HAS_BEEN_ENDED != retCode)
            {
                printf("error code = %d\n", retCode);
            }
            break;
        }
    }

    TRACEP("Command return Code: ", returnCode);
    TRACEP("Command output : ", output.c_str());
}
