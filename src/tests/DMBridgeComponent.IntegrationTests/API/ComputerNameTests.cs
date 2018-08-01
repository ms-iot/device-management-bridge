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
using System.Net;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Windows.Security.ExchangeActiveSyncProvisioning;

namespace DMBridgeComponent.IntegrationTests.API
{
    [TestClass]
    public class ComputerNameTests
    {
        private const string NEW_COMPUTERNAME_PREFIX = "settest";
        private ComputerNameBridge _compNameBridge;

        [TestInitialize]
        public void Initialize()
        {
            _compNameBridge = new ComputerNameBridge();
        }

        [TestCleanup]
        public void Cleanup()
        {
            _compNameBridge.Dispose();
            _compNameBridge = null;
        }

        [TestMethod]
        public void GetComputerName_NetBIOS_Equal()
        {
            // Arrange
            var netBIOSName = new EasClientDeviceInformation().FriendlyName;

            // Act
            string dmComputerName = _compNameBridge.GetName();

            // Assert
            Assert.AreEqual(netBIOSName, dmComputerName, true);
        }

        [TestMethod]
        public void GetComputerName_HostName_Equal()
        {
            // Arrange
            string hostName = Dns.GetHostName();

            // Act
            string dmComputerName = _compNameBridge.GetName();

            // Assert
            Assert.AreEqual(hostName, dmComputerName, true);
        }

        [TestMethod]
        public void IsComputerRenamePending_Equals_Eas()
        {
            // Arrange
            var currentNetBIOSName = new EasClientDeviceInformation().FriendlyName;
            var dmBridgeBIOSName = _compNameBridge.GetName();
            var isPending = (currentNetBIOSName != dmBridgeBIOSName);

            // Act
            var dmBringIsRenamePending = _compNameBridge.IsRenamePending();

            // Assert
            Assert.AreEqual(isPending, dmBringIsRenamePending);
        }

        [TestMethod]
        public void SetComputerName_NewName_RenamePendingIsTrue()
        {
            // Arrange
            Assert.IsFalse(_compNameBridge.IsRenamePending());
            var originalNetBIOSName = new EasClientDeviceInformation().FriendlyName;

            // Make sure the current NetBIOS name does not equal the new test one
            // If it does, alter the postfix on it
            string desiredNewName = $"{NEW_COMPUTERNAME_PREFIX}-1";
            if (String.Equals(
                    originalNetBIOSName,
                    desiredNewName, 
                    StringComparison.OrdinalIgnoreCase))
            {
                desiredNewName = $"{NEW_COMPUTERNAME_PREFIX}-2";
            }

            // Act
            _compNameBridge.SetName(desiredNewName);

            // Assert
            var isPending = _compNameBridge.IsRenamePending();
            _compNameBridge.SetName(originalNetBIOSName); // Revert name change
            Assert.IsTrue(isPending);          
        }
    }
}
