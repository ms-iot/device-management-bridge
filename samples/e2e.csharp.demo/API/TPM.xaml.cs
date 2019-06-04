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

using System;
using Windows.UI.Xaml.Navigation;

using DMBridgeComponent;

namespace e2e.csharp.demo.API
{
    public sealed partial class TPM : BaseAPIPage
    {
        private StatusViewModel TpmViewModel;
        private readonly TpmBridge _tpmBridge;

        public TPM()
        {
            TpmViewModel = new StatusViewModel("Loading...");
            this.InitializeComponent();
            _tpmBridge = new TpmBridge();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            UpdateTpmInfo();
        }

        private async void UpdateTpmInfo()
        {
            string ek = "";
            string regId = "";
            string connectionString = "";

            await base.RunAPIInBackgroundWithManualStatus(() => {

                try
                {
                    ek = _tpmBridge.GetEndorsementKey();
                }
                catch (Exception e)
                {
                    ek = e.Message;
                }

                try
                {
                    regId = _tpmBridge.GetRegistrationId();
                }
                catch (Exception e)
                {
                    regId = e.Message;
                }

                try
                {
                    connectionString = _tpmBridge.GetConnectionString(0, 3600);
                }
                catch (Exception e)
                {
                    connectionString = e.Message;
                }

                // Set TpmViewModel (and the bound UI) to be the current
                // value
                return ek;
            }, TpmViewModel);

            EndorsementKey.Text = ek;
            RegId.Text = regId;
            ConnectionString.Text = connectionString;
        }
    }
}
