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
#include <windows.h>
#include <filesystem>
#include <TraceLoggingProvider.h>
#include "ETWLogger.h"

#define PROVIDER_NAME "DMBridge"
// 16311AEF-09B6-471A-AD71-54A2756D5FEB
#define PROVIVDER_GUID (0x16311aef, 0x9b6, 0x471a, 0xad, 0x71, 0x54, 0xa2, 0x75, 0x6d, 0x5f, 0xeb)

TRACELOGGING_DEFINE_PROVIDER(gLogProvider, PROVIDER_NAME, PROVIVDER_GUID);

using namespace std;
using namespace experimental::filesystem;

#define WINEVENT_LEVEL_VERBOSE 5
#define WINEVENT_LEVEL_INFORMATION 4
#define WINEVENT_LEVEL_WARNING 3
#define WINEVENT_LEVEL_ERROR 2
#define WINEVENT_LEVEL_CRITICAL 1

namespace Utils
{
	ETWLogger::ETWLogger()
	{
		_exeFileName = GetExeFileName();
		TraceLoggingRegister(gLogProvider);
	}

	ETWLogger::~ETWLogger()
	{
		TraceLoggingUnregister(gLogProvider);
	}

	wstring ETWLogger::GetExeFileName()
	{
		wstring retValue;
		size_t size = MAX_PATH;

		do
		{
			vector<wchar_t> moduleFileName(size);
			::GetModuleFileName(NULL, moduleFileName.data(), static_cast<DWORD>(moduleFileName.size()));

			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				size *= 2;
				continue;
			}

			path p(moduleFileName.data());
			retValue = p.filename().c_str();
			break;

		} while (true);

		return retValue;
	}

	void ETWLogger::Log(const wstring& msg, LoggingLevel level)
	{
		switch (level)
		{
		case Verbose:
			TraceLoggingWrite(gLogProvider, "LogMsgVerbose",
				TraceLoggingLevel(WINEVENT_LEVEL_VERBOSE),
				TraceLoggingWideString(msg.c_str(), "msg"),
				TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
			break;
		case Information:
			TraceLoggingWrite(gLogProvider, "LogMsgInformation",
				TraceLoggingLevel(WINEVENT_LEVEL_INFORMATION),
				TraceLoggingWideString(msg.c_str(), "msg"),
				TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
			break;
		case Warning:
			TraceLoggingWrite(gLogProvider, "LogMsgWarning",
				TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
				TraceLoggingWideString(msg.c_str(), "msg"),
				TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
			break;
		case Error:
			TraceLoggingWrite(gLogProvider, "LogMsgError",
				TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
				TraceLoggingWideString(msg.c_str(), "msg"),
				TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
			break;
		case Critical:
			TraceLoggingWrite(gLogProvider, "LogMsgCritical",
				TraceLoggingLevel(WINEVENT_LEVEL_CRITICAL),
				TraceLoggingWideString(msg.c_str(), "msg"),
				TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
			break;
		}
	}

	void ETWLogger::Log(const string& msg, LoggingLevel level)
	{
		switch (level)
		{
		case Verbose:
			TraceLoggingWrite(gLogProvider, "LogMsgVerbose",
				TraceLoggingLevel(WINEVENT_LEVEL_VERBOSE),
				TraceLoggingString(msg.c_str(), "msg"),
				TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
			break;
		case Information:
			TraceLoggingWrite(gLogProvider, "LogMsgInformation",
				TraceLoggingLevel(WINEVENT_LEVEL_INFORMATION),
				TraceLoggingString(msg.c_str(), "msg"),
				TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
			break;
		case Warning:
			TraceLoggingWrite(gLogProvider, "LogMsgWarning",
				TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
				TraceLoggingString(msg.c_str(), "msg"),
				TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
			break;
		case Error:
			TraceLoggingWrite(gLogProvider, "LogMsgError",
				TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
				TraceLoggingString(msg.c_str(), "msg"),
				TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
			break;
		case Critical:
			TraceLoggingWrite(gLogProvider, "LogMsgCritical",
				TraceLoggingLevel(WINEVENT_LEVEL_CRITICAL),
				TraceLoggingString(msg.c_str(), "msg"),
				TraceLoggingWideString(_exeFileName.c_str(), "exeName"));
			break;
		}
	}
}