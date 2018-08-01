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
#include "NTServiceConfig.h"
#include "DMBridgeException.h"
#include "StringUtils.h"

constexpr char* NTServiceSection = "servicemanager";
constexpr char* WhitelistKey = "whitelist";

using namespace std;
using namespace Json;

NTServiceConfig::NTServiceConfig()
{
	TRACE(__FUNCTION__);
	ApplyDefaults();
}

NTServiceConfig::NTServiceConfig(const Json::Value& root)
{
	TRACE(__FUNCTION__);

	if (root.isNull() || !ParseJSON(root))
	{
		TRACE("Failed to use config, applying defaults");
		ApplyDefaults();
	}
}

void NTServiceConfig::ApplyDefaults()
{
	TRACE(__FUNCTION__);

	_whitelist = {
		L"w32time" /* Windows time */
	};
}

bool NTServiceConfig::ParseJSON(const Json::Value& root)
{
	TRACE(__FUNCTION__);

	if (root.isNull() || !root.isObject())
	{
		TRACE(L"Warning: Configuration is empty");
		return false;
	}

	set<wstring, Utils::CaseInsensitiveLess> newWhitelist;

	if (!root[WhitelistKey].isArray())
	{
		TRACE(L"Warning: Whitelist not defined");
		return false;
	}
	for (const Json::Value& serviceVal : root[WhitelistKey])
	{
		if (!serviceVal.isString())
		{
			TRACE(L"Warning: Non-string in whitelist");
			continue;
		}

		wstring serviceStr = Utils::MultibyteToWide(serviceVal.asString().c_str());
		if (serviceStr.size() == 0)
		{
			TRACE(L"Warning: Empty string in whitelist");
			continue;
		}

		TRACEP(L"Adding service to whitelist, if not already present: ", serviceStr);
		newWhitelist.insert(serviceStr);
	}

	_whitelist = newWhitelist;
	return true;
}
