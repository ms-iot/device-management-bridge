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
#include "DMBridgeConfig.h"
#include "DMBridgeException.h"
#include "StringUtils.h"

constexpr char* API_KEY = "api";

using namespace std;
using namespace Json;

DMBridgeConfig::DMBridgeConfig()
{
	TRACE(__FUNCTION__);
	ApplyDefaults();
}

DMBridgeConfig::DMBridgeConfig(const Json::Value& root)
{
	TRACE(__FUNCTION__);

	if (root.isNull() || !ParseJSON(root))
	{
		TRACE("Failed to use config, applying defaults");
		ApplyDefaults();
	}
}


map<wstring, RPC_IF_HANDLE, Utils::CaseInsensitiveLess> DMBridgeConfig::MakeInterfaceMap()
{
	TRACE(__FUNCTION__);

	map<wstring, RPC_IF_HANDLE, Utils::CaseInsensitiveLess> interfaceMap = {
		{ L"computername", ComputerName_v1_0_s_ifspec },
		{ L"servicemanager", NTService_v1_0_s_ifspec },
		{ L"telemetry", Telemetry_v1_0_s_ifspec}
	};

	return interfaceMap;
}


void DMBridgeConfig::ApplyDefaults()
{
	TRACE(__FUNCTION__);

	vector<RPC_IF_HANDLE> apiInterfaces;
	for (auto const &pair : _interfaceMap)
	{
		apiInterfaces.push_back(pair.second);
	}

	_enabledAPIIntefaces = apiInterfaces;
}

bool DMBridgeConfig::ParseJSON(const Json::Value& root)
{
	TRACE(__FUNCTION__);
	if (root.isNull() || !root.isObject())
	{
		TRACE(L"Warning: Configuration is empty");
		return false;
	}
	if (!root[API_KEY].isArray())
	{
		TRACE(L"Warning: API list not defined");
		return false;
	}

	set<RPC_IF_HANDLE> interfacesToEnable;
	for (const Json::Value& apiVal : root[API_KEY])
	{
		if (!apiVal.isString())
		{
			TRACE(L"Warning: Non-string in API list");
			continue;
		}
		wstring apiStr = Utils::MultibyteToWide(apiVal.asString().c_str());
		if (apiStr.size() == 0)
		{
			TRACE(L"Warning: Empty string in API list");
			continue;
		}

		auto interfaceSearch = _interfaceMap.find(apiStr);
		if (interfaceSearch == _interfaceMap.end())
		{
			TRACEP(L"Warning: Requested API is not available: ", apiStr);
			continue;
		}
		TRACEP(L"Registering API, if not already: ", apiStr);
		interfacesToEnable.insert(interfaceSearch->second);
	}

	_enabledAPIIntefaces.assign(interfacesToEnable.begin(), interfacesToEnable.end());

	return true;
}
