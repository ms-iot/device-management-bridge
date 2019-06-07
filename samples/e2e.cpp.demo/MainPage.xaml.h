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

#include "MainPage.g.h"

namespace e2e_cpp_demo
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
        struct DpsEnrollmentInfo
        {
            Platform::String^ registrationId;
            Platform::String^ endorsementKey;

            void Load();

            std::string ToJsonString() const;
            std::string ToSvgString() const;
        };

    public:
        MainPage();

    private:
        void OnReadTPMInfo(
            Platform::Object^ sender,
            Windows::UI::Xaml::RoutedEventArgs^ e);

        void OnSetServiceStartType(
            Platform::Object^ sender,
            Windows::UI::Xaml::RoutedEventArgs^ e);

        std::wstring WriteLocalFile(
            const std::string& content,
            const std::wstring& fileName);

        static DpsEnrollmentInfo GetDpsEnrollmentInfo();

        Windows::Foundation::IAsyncOperation<Windows::Storage::StorageFile^>^ _sampleFile;
    };
}
