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
using Windows.UI.Xaml.Navigation;
using System.Threading.Tasks;

using DMBridgeComponent;

namespace e2e.csharp.demo.API
{
    public enum TelemetryLevelOptions
    {
        SECURITY = 0,
        BASIC = 1,
        ENHANCED = 2,
        FULL = 3,
    }

    public sealed partial class TelemetryLevel : BaseAPIPage
    {
        private StatusViewModel TelemetryLevelViewModel;
        private readonly TelemetryLevelBridge _telemetryBridge;


        public TelemetryLevel()
        {
            TelemetryLevelViewModel = new StatusViewModel("Loading...");
            this.InitializeComponent();
            _telemetryBridge = new TelemetryLevelBridge();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            UpdateTelemetryLevelText();
        }

        private async Task UpdateTelemetryLevelText()
        {
            TelemetryLevelOptions level = 0;
            await base.RunAPIInBackgroundWithManualStatus(() => {
                level = (TelemetryLevelOptions)_telemetryBridge.GetLevel();
                // Set TelemetryLevelViewModel (and the bound UI) to be the current
                // telemetry level
                return level.ToString();
            }, TelemetryLevelViewModel);

            switch (level)
            {
                case TelemetryLevelOptions.SECURITY:
                    SecurityLevelRadioButton.IsChecked = true;
                    break;
                case TelemetryLevelOptions.BASIC:
                    BasicLevelRadioButton.IsChecked = true;
                    break;
                case TelemetryLevelOptions.ENHANCED:
                    EnhancedLevelRadioButton.IsChecked = true;
                    break;
                case TelemetryLevelOptions.FULL:
                    FullLevelRadioButton.IsChecked = true;
                    break;
                default:
                    break;
            }
        }

        private async void HandleLevelCheck(object sender, RoutedEventArgs e)
        {
            int level;

            if (SecurityLevelRadioButton.IsChecked == true)
                level = 0;
            else if (BasicLevelRadioButton.IsChecked == true)
                level = 1;
            else if (EnhancedLevelRadioButton.IsChecked == true)
                level = 2;
            else if (FullLevelRadioButton.IsChecked == true)
                level = 3;
            else
                return;

            await base.RunAPIInBackground(() => {
                _telemetryBridge.SetLevel(level);
            });
            await UpdateTelemetryLevelText();
        }
    }
}