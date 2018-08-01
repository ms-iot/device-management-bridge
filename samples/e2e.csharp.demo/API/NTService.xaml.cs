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

using Windows.UI.Xaml;
using e2e.csharp.demo.API;
using DMBridgeComponent;

namespace e2e.csharp.demo.API
{

    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms685996(v=vs.85).aspx
    public enum ServiceStatus
    {
        SERVICE_CONTINUE_PENDING = 0x00000005,
        SERVICE_PAUSE_PENDING = 0x00000006,
        SERVICE_PAUSED = 0x00000007,
        SERVICE_RUNNING = 0x00000004,
        SERVICE_START_PENDING = 0x00000002,
        SERVICE_STOP_PENDING = 0x00000003,
        SERVICE_STOPPED = 0x00000001
    }

    public sealed partial class NTService : BaseAPIPage
    {
        private readonly NTServiceBridge _ntServiceBridge;

        public NTService() : base()
        {
            _ntServiceBridge = new NTServiceBridge();
            this.InitializeComponent();
        }

        private async void StartButton_Click(object sender, RoutedEventArgs e)
        {
            var service = ServiceName.Text;
            await base.RunAPIInBackground(() => { _ntServiceBridge.Start(service); });
        }

        private async void StopButton_Click(object sender, RoutedEventArgs e)
        {
            var service = ServiceName.Text;
            await base.RunAPIInBackground(() => { _ntServiceBridge.Stop(service); });
        }

        private async void QueryButton_Click(object sender, RoutedEventArgs e)
        {
            var service = ServiceName.Text;
            await base.RunAPIInBackgroundWithManualStatus(() => {
                var status = (ServiceStatus)_ntServiceBridge.Query(service);
                return status.ToString();
            });
        }
    }
}
