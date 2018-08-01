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
#include "ConfigUtils.h"
#include "Constants.h"
#include "RegistryUtils.h"

using namespace std;
using namespace Json;

namespace ConfigUtils
{
	const Json::Value SafelyGetSection(const Json::Value& root, const std::string& section)
	{
		TRACE(__FUNCTION__);
		try
		{
			return root[section];
		}
		catch (const Json::LogicError& e)
		{
			TRACEP(L"Failed to get JSON section. Error: ", e.what());
		}

		return new Json::Value();
	}

	const Value LoadConfigFile()
	{
		TRACE(__FUNCTION__);

		wstring file = DefaultConfigFile;
		if (Utils::TryReadRegistryValue(IoTDMRegistryRoot, RegConfigFile, file) == ERROR_SUCCESS)
		{
			if (file.length() == 0)
			{
				file = DefaultConfigFile;
			}
		}

		try
		{
			return ParseJSONFile(file);
		}
		catch (const std::runtime_error& e)
		{
			TRACEP(L"Failed to open config file. Error: ", e.what());
		}
		catch (const Json::RuntimeError& e)
		{
			TRACEP(L"Failed to parse config file. Error: ", e.what());
		}
		catch (...)
		{
			TRACEP(L"Config error. Unknown exception caught. Error: ", GetLastError());
		}

		return new Json::Value();
	}

	const Value ParseJSONFile(const wstring& file)
	{
		TRACE(__FUNCTION__);

		if (file.length() == 0)
		{
			throw invalid_argument("No file path set");
		}

		TRACEP(L"Loading config file: ", file);

		CharReaderBuilder builder;
		ifstream jsonIfStream(file, ifstream::binary);
		if (!jsonIfStream.good())
		{
			throw runtime_error("Failed to open config file");
		}
		string errs;
		Value root;

		if (!Json::parseFromStream(builder, jsonIfStream, &root, &errs))
		{
			Json::throwRuntimeError(errs);
		}

		return root;
	}
}