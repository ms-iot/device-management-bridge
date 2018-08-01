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
#include "IConfig.h"
#include "StringUtils.h"

class DMBridgeConfig : public IConfig
{
public:
	DMBridgeConfig();
	DMBridgeConfig(const Json::Value& root);

	std::vector<RPC_IF_HANDLE> GetAPIInterfaces() const
	{
		return _enabledAPIIntefaces;
	}

private:
	std::map<std::wstring, RPC_IF_HANDLE, Utils::CaseInsensitiveLess> MakeInterfaceMap();
	bool ParseJSON(const Json::Value& root);
	void ApplyDefaults();

	const std::map<std::wstring, RPC_IF_HANDLE, Utils::CaseInsensitiveLess> _interfaceMap = MakeInterfaceMap();
	std::vector<RPC_IF_HANDLE> _enabledAPIIntefaces;
};