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
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DMBridgeComponent.IntegrationTests.API
{
    [TestClass]
    public class TelemetryLevelTests
    {
        private const int MAX_TELEMETRY_LEVEL = 3;
        private TelemetryLevelBridge _telemetryBridge;

        [TestInitialize]
        public void Initialize()
        {
            _telemetryBridge = new TelemetryLevelBridge();
        }

        [TestCleanup]
        public void Cleanup()
        {
            _telemetryBridge.Dispose();
        }

        [TestMethod]
        public void SetLevel_NewValue_EqualsGetValue()
        {
            // Arrange
            var originalLevel = _telemetryBridge.GetLevel();
            var targetLevel =  ((originalLevel + 1) % MAX_TELEMETRY_LEVEL);

            // Act
            _telemetryBridge.SetLevel(targetLevel);

            // Assert
            var newLevel = _telemetryBridge.GetLevel();
            Assert.AreEqual(targetLevel, newLevel);
        }
    }
}
