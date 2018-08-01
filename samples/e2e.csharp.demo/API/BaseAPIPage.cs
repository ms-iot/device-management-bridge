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
using Windows.UI.Xaml.Controls;
using System.Threading.Tasks;

namespace e2e.csharp.demo.API
{
    public class BaseAPIPage : Page
    {
        public StatusViewModel ViewModel { get; set; }

        public BaseAPIPage()
        {
            ViewModel = new StatusViewModel();
        }

        protected async Task RunAPIInBackground(Action toRun, StatusViewModel toUpdate = null)
        {
            await RunAPIInBackgroundWithManualStatus(() =>
            {
                toRun();
                return null;
            }, toUpdate);
        }

        protected async Task RunAPIInBackgroundWithManualStatus(Func<string> toRun, StatusViewModel toUpdate = null)
        {
            if (toUpdate == null)
                toUpdate = ViewModel;

            string statusUpdate;
            try
            {
                statusUpdate = await Task.Run(() => { return toRun(); });
                if (string.IsNullOrWhiteSpace(statusUpdate))
                    statusUpdate = "Success";
            }
            catch (Exception ex)
            {
                statusUpdate = ex.Message?.Split('\r')[0];
            }
            toUpdate.Status = statusUpdate;
        }
    }
}
