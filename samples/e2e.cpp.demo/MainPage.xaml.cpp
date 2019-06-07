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

#include "pch.h"
#include "MainPage.xaml.h"
#include "..\..\deps\QR-Code-generator\cpp\BitBuffer.hpp"
#include "..\..\deps\QR-Code-generator\cpp\QrCode.hpp"

using std::uint8_t;
using qrcodegen::QrCode;
using qrcodegen::QrSegment;

using namespace e2e_cpp_demo;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Navigation;

using namespace std;

using namespace DMBridgeComponent;

// https://docs.microsoft.com/en-us/windows/desktop/api/winsvc/nf-winsvc-changeserviceconfiga
enum ServiceStartType
{
    SERVICE_START_AUTO = 0x00000002,
    SERVICE_START_BOOT = 0x00000000,
    SERVICE_START_DEMAND = 0x00000003,
    SERVICE_DISABLED_ = 0x00000004,
    SERVICE_START_SYSTEM = 0x00000001
};

wstring MultibyteToWide(const char* s)
{
    size_t length = s ? strlen(s) : 0;
    size_t requiredCharCount = MultiByteToWideChar(CP_UTF8, 0, s, static_cast<int>(length), nullptr, 0);

    // add room for \0
    ++requiredCharCount;

    vector<wchar_t> wideString(requiredCharCount);
    MultiByteToWideChar(CP_UTF8, 0, s, static_cast<int>(length), wideString.data(), static_cast<int>(wideString.size()));

    return wstring(wideString.data());
}

string WideToMultibyte(const wchar_t* s)
{
    size_t length = s ? wcslen(s) : 0;
    size_t requiredCharCount = WideCharToMultiByte(CP_UTF8, 0, s, static_cast<int>(length), nullptr, 0, nullptr, nullptr);

    // add room for \0
    ++requiredCharCount;

    vector<char> multibyteString(requiredCharCount);
    WideCharToMultiByte(CP_UTF8, 0, s, static_cast<int>(length), multibyteString.data(), static_cast<int>(multibyteString.size()), nullptr, nullptr);

    return string(multibyteString.data());
}

MainPage::MainPage()
{
	InitializeComponent();
}

void MainPage::DpsEnrollmentInfo::Load()
{
    TpmBridge^ tpmBridge = ref new TpmBridge();

    registrationId = tpmBridge->GetRegistrationId();
    endorsementKey = tpmBridge->GetEndorsementKey();
}

std::string MainPage::DpsEnrollmentInfo::ToJsonString() const
{
    string s;

    s += "{";
    s += "\"regid\":\"";
    s += WideToMultibyte(registrationId->Data());
    s += "\",\"ek\":\"";
    s += WideToMultibyte(endorsementKey->Data());
    s += "\"";
    s += "}";

    return s;
}

std::string MainPage::DpsEnrollmentInfo::ToSvgString() const
{
    const QrCode::Ecc errCorLvl = QrCode::Ecc::LOW;
    const QrCode qr = QrCode::encodeText(ToJsonString().c_str(), errCorLvl);
    return qr.toSvgString(4 /*border*/);
}

wstring MainPage::WriteLocalFile(
    const string& content,
    const wstring& fileName)
{
    auto filePath = wstring(Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data()) + L"\\" + fileName;
    HANDLE fileHandle = CreateFile2(filePath.c_str(), GENERIC_WRITE, 0, CREATE_ALWAYS, nullptr);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        auto writeResult = WriteFile(fileHandle, content.data(), static_cast<DWORD>(content.length()), &bytesWritten, nullptr);
        if (writeResult == FALSE || bytesWritten != content.length())
        {
            throw runtime_error("Failed to write content message to the content file: error code " + to_string(GetLastError()));
        }
        CloseHandle(fileHandle);
    }

    return filePath;
}

void MainPage::OnReadTPMInfo(
    Platform::Object^ sender,
    Windows::UI::Xaml::RoutedEventArgs^ e)
{
    // Load the data from the TPM...
    DpsEnrollmentInfo info;
    info.Load();

    // Load the data in the UI...
    RegIdBox->Text = info.registrationId;
    EKBox->Text = info.endorsementKey;

    // Save it to an image...
    wstring fullFileName = WriteLocalFile(info.ToSvgString(), L"qrd.svg");

    // Load the image in the UI...
    Uri^ uri = ref new Uri(ref new Platform::String(fullFileName.c_str()));
    SvgImageSource^ svgImageSource = ref new SvgImageSource(uri);
    QrImage->Source = svgImageSource;
}

void e2e_cpp_demo::MainPage::OnSetServiceStartType(
    Platform::Object^ sender,
    Windows::UI::Xaml::RoutedEventArgs^ e)
{
    NTServiceBridge^ ntServiceBridge = ref new NTServiceBridge();

    int startType = 0;
    switch (ServiceStartTypeCombobox->SelectedIndex)
    {
    case 0: // auto
        startType = (int)ServiceStartType::SERVICE_START_AUTO;
        break;
    case 1:
        startType = (int)ServiceStartType::SERVICE_START_BOOT;
        break;
    case 2:
        startType = (int)ServiceStartType::SERVICE_START_DEMAND;
        break;
    case 3:
        startType = (int)ServiceStartType::SERVICE_START_SYSTEM;
        break;
    case 4:
        startType = (int)ServiceStartType::SERVICE_DISABLED_;
        break;
    }

    try
    {
        StatusBox->Text = "...";
        ntServiceBridge->SetStartMode(ServiceNameBox->Text, startType);
        StatusBox->Text = "Success";
    }
    catch (Platform::Exception^ e)
    {
        StatusBox->Text = e->Message;
    }
}
