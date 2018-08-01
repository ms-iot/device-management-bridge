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
#include "DMBridgeException.h"
#include "DMBridgeServer.h"
#include "RpcConstants.h"


// The capability that UWP apps must have inorder to be able to use the Rpc endpoints
constexpr wchar_t* REQUIRED_CAPABILITY = L"systemManagement";
constexpr int RPC_MIN_CALLS = 1;
constexpr int RPC_DONT_WAIT = 0;

std::unique_ptr<DMBridgeConfig> DMBridgeServer::_config;

using namespace std;

void DMBridgeServer::Setup()
{
	TRACE(__FUNCTION__);


	SECURITY_DESCRIPTOR rpcSecurityDescriptor;
	RPC_STATUS status = RPC_S_OK;

	try
	{
		rpcSecurityDescriptor = GenerateSecurityDescriptor(REQUIRED_CAPABILITY);
	}
	catch (const DMBridgeExceptionWithErrorCode)
	{
		TRACE(L"Error: Failed to generate security descriptor.");
		throw;
	}
	catch (...)
	{
		TRACEP(
			L"Error: Failed to generate security descriptor, unhandled exception. Error ",
			GetLastError());
		throw;
	}

	status = RpcServerUseProtseqEp(
		(RPC_WSTR)RPC_PROTOCOL_SEQUENCE,
		RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
		(RPC_WSTR)RPC_ENDPOINT,
		&rpcSecurityDescriptor);

	if (status != RPC_S_OK)
	{
		throw DMBridgeExceptionWithErrorCode("Failed to setup RPC protocol", status);
	}
	
	try
	{
		if (_config == nullptr)
		{
			TRACE(L"Error: Null configuration, cannot register interfaces.");
			return;
		}
		auto interfaces = _config->GetAPIInterfaces();
		RegisterInterfaces(&rpcSecurityDescriptor, interfaces);
	}
	catch (const DMBridgeExceptionWithErrorCode)
	{
		TRACE(L"Error: Failed to register interfaces.");
		throw;
	}
	catch (...)
	{
		TRACEP(
			L"Error: Failed to register interfaces, unhandled exception. Error ",
			GetLastError());
		throw;
	}
}

void DMBridgeServer::Listen()
{
	TRACE(__FUNCTION__);
	RPC_STATUS status = RPC_S_OK;

	status = RpcServerListen(
		RPC_MIN_CALLS,
		RPC_C_LISTEN_MAX_CALLS_DEFAULT,
		RPC_DONT_WAIT);
	
	if (status != RPC_S_OK)
	{
		throw DMBridgeExceptionWithErrorCode("Failed to listen for RPC", status);
	}
}

void DMBridgeServer::StopListening()
{
	TRACE(__FUNCTION__);
	RPC_STATUS status = RPC_S_OK;

	status = RpcMgmtStopServerListening(NULL /* Stop this program's RPC binding*/);

	if (status != RPC_S_OK)
	{
		throw DMBridgeExceptionWithErrorCode("Failed to stop listening for RPC", status);
	}
}

void DMBridgeServer::RegisterInterfaces(
	SECURITY_DESCRIPTOR* securityDescriptor,
	const std::vector<RPC_IF_HANDLE>& interfaces)
{
	TRACE(__FUNCTION__);
	TRACEP("Number of interfaces: ", interfaces.size());
	RPC_STATUS status = RPC_S_OK;
	for (RPC_IF_HANDLE rpcInterface : interfaces)
	{
		status = RPC_S_OK;
		status = RpcServerRegisterIf3(
			rpcInterface,
			nullptr,
			nullptr,
			RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_LOCAL_ONLY,
			RPC_C_LISTEN_MAX_CALLS_DEFAULT,
			0,
			nullptr,
			securityDescriptor);

		if (status != RPC_S_OK)
		{
			throw DMBridgeExceptionWithErrorCode("Failed to register interface", status);
		}
	}
}

SECURITY_DESCRIPTOR DMBridgeServer::GenerateSecurityDescriptor(const WCHAR* capability)
{
	TRACE(__FUNCTION__);
	TRACEP(L"Requiring capability: ", capability);
	// Security Policy
	DWORD hResult = S_OK;
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
	PSID everyoneSid = nullptr;
	PSID* capabilitySids = nullptr;
	DWORD capabilitySidCount = 0;
	PSID* capabilityGroupSids = nullptr;
	DWORD capabilityGroupSidCount = 0;
	EXPLICIT_ACCESS ea[2] = {};
	PACL acl = nullptr;
	SECURITY_DESCRIPTOR rpcSecurityDescriptor = {};

	// Get the SID form of the custom capability.  In this case we only expect one SID and
	// we don't care about the capability group. 
	if (!DeriveCapabilitySidsFromName(
		capability,
		&capabilityGroupSids,
		&capabilityGroupSidCount,
		&capabilitySids,
		&capabilitySidCount))
	{
		throw DMBridgeExceptionWithErrorCode("Failed to derive capability sids from name", GetLastError());
	}

	if (capabilitySidCount != 1)
	{
		throw DMBridgeExceptionWithErrorCode("Unexpected sid count", ERROR_INVALID_PARAMETER);
	}

	if (!AllocateAndInitializeSid(
		&SIDAuthWorld,
		1,
		SECURITY_WORLD_RID,
		0, 0, 0, 0, 0, 0, 0,
		&everyoneSid))
	{
		throw DMBridgeExceptionWithErrorCode("Failed to allocate and initialize sid", GetLastError());
	}

	// Everyone GENERIC_ALL access
	ea[0].grfAccessMode = SET_ACCESS;
	ea[0].grfAccessPermissions = GENERIC_ALL;
	ea[0].grfInheritance = NO_INHERITANCE;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName = static_cast<LPWSTR>(everyoneSid);
	// Custom capability GENERIC_ALL access
	ea[1].grfAccessMode = SET_ACCESS;
	ea[1].grfAccessPermissions = GENERIC_ALL;
	ea[1].grfInheritance = NO_INHERITANCE;
	ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[1].Trustee.TrusteeType = TRUSTEE_IS_UNKNOWN;
	// Earlier we ensured there was exactly 1
	ea[1].Trustee.ptstrName = static_cast<LPWSTR>(capabilitySids[0]);

	hResult = SetEntriesInAcl(ARRAYSIZE(ea), ea, nullptr, &acl);
	if (hResult != ERROR_SUCCESS)
	{
		throw DMBridgeExceptionWithErrorCode("Failed to set entries in Acl", hResult);
	}

	if (!InitializeSecurityDescriptor(&rpcSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION))
	{
		throw DMBridgeExceptionWithErrorCode("Failed to initialize security descriptor", GetLastError());
	}

	if (!SetSecurityDescriptorDacl(&rpcSecurityDescriptor, TRUE, acl, FALSE))
	{
		throw DMBridgeExceptionWithErrorCode("Failed to set security descriptor Dacl", GetLastError());
	}

	return rpcSecurityDescriptor;
}

/******************************************************/
/*         MIDL allocate and free                     */
/******************************************************/

void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
	return(malloc(len));
}

void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
	free(ptr);
}