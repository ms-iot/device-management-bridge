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
#include "Tpm.h"
#include "DMBridgeException.h"
#include "Logger.h"
#include "DMProcess.h"
#include "StringUtils.h"
#include "../SharedUtilities/json/json.h"

using namespace std;

constexpr wchar_t GetTpmInfoCmd[] = L"Limpet.exe -azuredps -enrollmentinfo -json";

constexpr char JsonAttestation[] = "attestation";
constexpr char JsonTpm[] = "tpm";
constexpr char JsonEK[] = "endorsementKey";
constexpr char JsonRegId[] = "registrationId";

/* Map Generated rpc method signatures to class */
/* -------------------------------------------- */

HRESULT GetEndorsementKeyRpc(_In_ handle_t, _Outptr_ int *size, _Outptr_ wchar_t **ek)
{
    return Tpm::GetEndorsementKey(*size, *ek);
}
HRESULT GetRegistrationIdRpc(_In_ handle_t, _Outptr_ int *size, _Outptr_ wchar_t **regId)
{
    return Tpm::GetRegistrationId(*size, *regId);
}
HRESULT GetConnectionStringRpc(_In_ handle_t, INT32 slot, int expiryInSeconds, _Outptr_ int *size, _Outptr_ wchar_t **cs)
{
    return Tpm::GetConnectionString(slot, expiryInSeconds, *size, *cs);
}
/* -------------------------------------------- */

Json::Value JsonObjectFromString(const string& stringValue)
{
    istringstream payloadStream(stringValue);
    Json::Value root;
    string errorsList;
    Json::CharReaderBuilder builder;
    if (!Json::parseFromStream(builder, payloadStream, &root, &errorsList))
    {
        throw exception("Failed to parse json.");
    }
    return root;
}

HRESULT TpmInfoFromLimpet(
    const string& output,
    string& ek,
    string& regId)
{
    Json::Value jsonArray = JsonObjectFromString(output);
    if (jsonArray.isNull() || !jsonArray.isArray())
    {
        return E_FAIL;
    }

    Json::Value jsonObject;

    for (Json::Value::const_iterator it = jsonArray.begin(); it != jsonArray.end(); ++it)
    {
        // One element only...
        jsonObject = *it;
        break;
    }

    if (jsonObject.isNull() || !jsonObject.isObject())
    {
        return E_FAIL;
    }

    Json::Value jsonAttestation = jsonObject[JsonAttestation];
    if (jsonAttestation.isNull() || !jsonAttestation.isObject())
    {
        return E_FAIL;
    }

    Json::Value jsonTpm = jsonAttestation[JsonTpm];
    if (jsonTpm.isNull() || !jsonTpm.isObject())
    {
        return E_FAIL;
    }
    Json::Value jsonEK = jsonTpm[JsonEK];
    if (!jsonEK.isString())
    {
        return E_FAIL;
    }
    ek = jsonEK.asString();

    Json::Value jsonRegId = jsonObject[JsonRegId];
    if (!jsonRegId.isString())
    {
        return E_FAIL;
    }
    regId = jsonRegId.asString();

    return S_OK;
}

string Tpm::RunLimpet(const wstring& params)
{
    TRACE(__FUNCTION__);

    string output;

    // build limpet command and invoke it  
    wchar_t sys32dir[MAX_PATH];
    GetSystemDirectoryW(sys32dir, _countof(sys32dir));

    wchar_t fullCommand[MAX_PATH];
    swprintf_s(fullCommand, _countof(fullCommand), L"%s\\%s %s", sys32dir, L"limpet.exe", params.c_str());

    unsigned long returnCode;

    Process::Launch(fullCommand, returnCode, output);

    return output;
}

HRESULT Tpm::GetHostNameAndDeviceId(int logicalId, string& serviceUrl)
{
    TRACE(__FUNCTION__);

    const string response = RunLimpet(to_wstring(logicalId) + L" -rur");

    regex rgx(".*<ServiceURI>\\s*(\\S+)\\s*</ServiceURI>.*");
    smatch match;

    if (regex_search(response.begin(), response.end(), match, rgx))
    {
        serviceUrl = match[1];
        return S_OK;
    }
    return E_FAIL;
}

HRESULT Tpm::GetSASToken(int logicalId, unsigned int durationInSeconds, string& sasToken)
{
    TRACE(__FUNCTION__);

    const string response = RunLimpet(to_wstring(logicalId) + L" -ast " + to_wstring(durationInSeconds));

    // There is a bug in Limpet that produces the entire connection string and not only the SAS token
    // Work around by extracting the actual connection string
    // The workaround will continue to work (but will be unnecessary) once the bug in Limpet is fixed

    regex rgx(".*(SharedAccessSignature sr.*)");
    smatch match;

    if (regex_search(response.begin(), response.end(), match, rgx))
    {
        sasToken = match[1];
        return S_OK;
    }
    return E_FAIL;
}

HRESULT Tpm::WriteRpcOutputString(const wstring& value, _Outptr_ int &rawValueSize, _Outptr_ wchar_t *&rawValue)
{
    TRACE(__FUNCTION__);

    rawValueSize = static_cast<int>(value.size()) + 1;

    rawValue = (wchar_t*)midl_user_allocate(rawValueSize * sizeof(wchar_t));
    if (rawValue == NULL)
    {
        TRACE("Failed to get computer name. Could not allocate memory.");
        return E_OUTOFMEMORY;
    }

    errno_t copyErr = wcscpy_s(rawValue, rawValueSize, value.c_str());
    if (copyErr != 0)
    {
        TRACEP("Failed to get computer name. Could not copy buffer to out pointer. Errno: ", copyErr);
        return E_FAIL;
    }

    return S_OK;
}

HRESULT Tpm::GetEndorsementKey(_Outptr_ int &size, _Outptr_ wchar_t *&ek)
{
    TRACE(__FUNCTION__);

    const string output = RunLimpet(L" -azuredps -enrollmentinfo -json");

    string ekStr;
    string regIdStr;
    HRESULT hr = TpmInfoFromLimpet(output, ekStr, regIdStr);
    if (FAILED(hr))
    {
        return hr;
    }

    return WriteRpcOutputString(Utils::MultibyteToWide(ekStr.c_str()), size, ek);
}

HRESULT Tpm::GetRegistrationId(_Outptr_ int &size, _Outptr_ wchar_t *&regId)
{
    TRACE(__FUNCTION__);

    const string output = RunLimpet(L" -azuredps -enrollmentinfo -json");

    string ekStr;
    string regIdStr;
    HRESULT hr = TpmInfoFromLimpet(output, ekStr, regIdStr);
    if (FAILED(hr))
    {
        return hr;
    }

    return WriteRpcOutputString(Utils::MultibyteToWide(regIdStr.c_str()), size, regId);
}

HRESULT Tpm::GetConnectionString(_In_ int slot, _In_ int expiryInSeconds, _Outptr_ int &size, _Outptr_ wchar_t *&cs)
{
    TRACE(__FUNCTION__);

    string hostAndDeviceId;
    HRESULT hr = GetHostNameAndDeviceId(slot, hostAndDeviceId);
    if (FAILED(hr))
    {
        return hr;
    }

    vector<string> hostAndDeviceIdParts;
    Utils::SplitString(hostAndDeviceId, '/', hostAndDeviceIdParts);
    if (hostAndDeviceIdParts.size() != 2)
    {
        return E_FAIL;
    }

    string hostName = hostAndDeviceIdParts[0];
    string deviceId = hostAndDeviceIdParts[1];

    string sasToken;
    hr = GetSASToken(slot, expiryInSeconds, sasToken);
    if (FAILED(hr))
    {
        return hr;
    }

    string connectionString;
    connectionString += "HostName=";
    connectionString += hostName;
    connectionString += ";DeviceId=";
    connectionString += deviceId;
    connectionString += ";SharedAccessSignature=";
    connectionString += sasToken;

    return WriteRpcOutputString(Utils::MultibyteToWide(connectionString.c_str()), size, cs);
}
