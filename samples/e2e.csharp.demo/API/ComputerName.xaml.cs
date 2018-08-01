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
using Windows.UI.Xaml;
using Windows.UI.Xaml.Navigation;

using DMBridgeComponent;

namespace e2e.csharp.demo.API
{
    public sealed partial class ComputerName : BaseAPIPage
    {
        private StatusViewModel ComputerNameViewModel;
        private readonly ComputerNameBridge _compNameBridge;


        public ComputerName()
        {
            ComputerNameViewModel = new StatusViewModel("Loading...");
            _compNameBridge = new ComputerNameBridge();

            this.InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            UpdateComputerNameText();
        }

        private async void UpdateComputerNameText()
        {
            await base.RunAPIInBackgroundWithManualStatus(() => {
                return _compNameBridge.GetName();
            }, ComputerNameViewModel);
        }

        private async void RenameButton_Click(object sender, RoutedEventArgs e)
        {
            var name = NewComputerName.Text;
            await base.RunAPIInBackground(() => {
                _compNameBridge.SetName(name);
            });
        }
    }
}
