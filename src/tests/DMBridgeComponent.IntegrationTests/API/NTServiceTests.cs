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
using System.Threading.Tasks;
using DMBridgeComponent;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DMBridgeComponent.IntegrationTests.API
{
    [TestClass]
    public class NTServiceTests
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

        private const string NON_WHITELISTED_SERVICE = "dhcp";
        private const string WHITELISTED_SERVICE = "w32time";
        private const int STATUS_CHANGE_WAIT_MS = 500;

        private NTServiceBridge _ntServiceBridge;

        [TestInitialize]
        public void Initialize()
        {
            _ntServiceBridge = new NTServiceBridge();
        }

        [TestCleanup]
        public void Cleanup()
        {
            _ntServiceBridge.Dispose();
            _ntServiceBridge = null;
        }

        #region WhitelistedService
        [TestMethod]
        [ExpectedException(typeof(UnauthorizedAccessException))]
        public void Start_NonWhitelistedService_ExceptionThrown()
        {
            // Act
            _ntServiceBridge.Start(NON_WHITELISTED_SERVICE);
        }

        [TestMethod]
        [ExpectedException(typeof(UnauthorizedAccessException))]
        public void Stop_NonWhitelistedService_ExceptionThrown()
        {
            // Act
            _ntServiceBridge.Stop(NON_WHITELISTED_SERVICE);
        }

        [TestMethod]
        public void Query_NonWhitelistedService_NoExceptionThrown()
        {
            //Act
            _ntServiceBridge.Query(NON_WHITELISTED_SERVICE);
        }

        [TestMethod]
        [ExpectedException(typeof(UnauthorizedAccessException))]
        public void Start_NonWhitelistedServiceCaseInsensitive_ExceptionThrown()
        {
            // Act
            _ntServiceBridge.Start(NON_WHITELISTED_SERVICE.ToUpper());
        }

        [TestMethod]
        [ExpectedException(typeof(UnauthorizedAccessException))]
        public void Stop_NonWhitelistedServiceCaseInsensitive_ExceptionThrown()
        {
            // Act
            _ntServiceBridge.Stop(NON_WHITELISTED_SERVICE.ToUpper());
        }

        [TestMethod]
        public void Query_NonWhitelistedServiceCaseInsensitive_NoExceptionThrown()
        {
            // Act
            _ntServiceBridge.Query(NON_WHITELISTED_SERVICE.ToUpper());
        }
        #endregion

        [TestMethod]
        public void Start_AlreadyStarted_NoExceptionThrown()
        {
            // Arrange
            _ntServiceBridge.Start(WHITELISTED_SERVICE);
            Task.Delay(STATUS_CHANGE_WAIT_MS).Wait();

            // Act
            _ntServiceBridge.Start(WHITELISTED_SERVICE);
        }

        [TestMethod]
        public void Stop_AlreadyStopped_NoExceptionThrown()
        {
            // Arrange
            _ntServiceBridge.Stop(WHITELISTED_SERVICE);
            Task.Delay(STATUS_CHANGE_WAIT_MS).Wait();

            // Act
            _ntServiceBridge.Stop(WHITELISTED_SERVICE);
        }

        [TestMethod]
        public void Query_RunningService_CorrectStatus()
        {
            // Arrange
            _ntServiceBridge.Start(WHITELISTED_SERVICE);
            Task.Delay(STATUS_CHANGE_WAIT_MS).Wait();

            // Act
            var rawStatus = _ntServiceBridge.Query(WHITELISTED_SERVICE);
            var status = (ServiceStatus)rawStatus;
            Assert.IsTrue(
                (status == ServiceStatus.SERVICE_START_PENDING) ||
                (status == ServiceStatus.SERVICE_RUNNING));
        }

        [TestMethod]
        public void Query_StoppedService_CorrectStatus()
        {
            // Arrange
            _ntServiceBridge.Stop(WHITELISTED_SERVICE);
            Task.Delay(STATUS_CHANGE_WAIT_MS).Wait();

            // Act
            var rawStatus = _ntServiceBridge.Query(WHITELISTED_SERVICE);
            var status = (ServiceStatus)rawStatus;
            Assert.IsTrue(
                (status == ServiceStatus.SERVICE_STOP_PENDING) ||
                (status == ServiceStatus.SERVICE_STOPPED));
        }

        [TestMethod]
        public void Stop_RunningService_CorrectStatus()
        {
            // Arrange
            _ntServiceBridge.Start(WHITELISTED_SERVICE);
            Task.Delay(STATUS_CHANGE_WAIT_MS).Wait();

            // Make sure service is running
            var rawStatus = _ntServiceBridge.Query(WHITELISTED_SERVICE);
            var status = (ServiceStatus)rawStatus;
            Assert.IsTrue(
                (status == ServiceStatus.SERVICE_START_PENDING) ||
                (status == ServiceStatus.SERVICE_RUNNING));

            // Attempt to stop it
            _ntServiceBridge.Stop(WHITELISTED_SERVICE);
            Task.Delay(STATUS_CHANGE_WAIT_MS * 2).Wait();

            // Check if stop worked
            rawStatus = _ntServiceBridge.Query(WHITELISTED_SERVICE);
            status = (ServiceStatus)rawStatus;
            Assert.IsTrue(
                (status == ServiceStatus.SERVICE_STOP_PENDING) ||
                (status == ServiceStatus.SERVICE_STOPPED));
        }

        [TestMethod]
        public void Start_StoppedService_CorrectStatus()
        {
            // Arrange
            _ntServiceBridge.Stop(WHITELISTED_SERVICE);
            Task.Delay(STATUS_CHANGE_WAIT_MS).Wait();

            // Make sure service is stopped
            var rawStatus = _ntServiceBridge.Query(WHITELISTED_SERVICE);
            var status = (ServiceStatus)rawStatus;
            Assert.IsTrue(
                (status == ServiceStatus.SERVICE_STOP_PENDING) ||
                (status == ServiceStatus.SERVICE_STOPPED));

            // Attempt to start it
            _ntServiceBridge.Start(WHITELISTED_SERVICE);
            Task.Delay(STATUS_CHANGE_WAIT_MS * 2).Wait();

            // Check if start worked
            rawStatus = _ntServiceBridge.Query(WHITELISTED_SERVICE);
            status = (ServiceStatus)rawStatus;
            Assert.IsTrue(
                (status == ServiceStatus.SERVICE_START_PENDING) ||
                (status == ServiceStatus.SERVICE_RUNNING));
        }
    }
}
