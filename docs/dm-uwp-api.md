# DM Bridge UWP API

- [Referencing](#referencing)
    - [UWP Capability](#uwp-capability)
    - [Using the NuGet Package](#using-the-nuget-package)
- [Thrown Exceptions](#thrown-exceptions)
- [API Documentation](#api-documentation)

## Referencing
UWP Applications can consume the DM Bridge APIs either by referencing the
`DMBridgeComponenet` project or a NuGet package of that project.

### UWP Capability

In order for a UWP application to use `DMBridgeComponent`, it must have the
`SystemManagement` capability set. This can be done by opening the
`Package.appxmanifest` file in Visual Studio and going to the `Capabilities`
tab, or by manually editing the `Package.appxmanifest` file associated with the
project and adding:

```xml
<Package
xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
xmlns:mp="http://schemas.microsoft.com/appx/2014/phone/manifest"
xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10"
xmlns:iot="http://schemas.microsoft.com/appx/manifest/iot/windows10"
IgnorableNamespaces="uap mp iot">
  ...
  <Capabilities>
    <iot:Capability Name="systemManagement" />
  </Capabilities>
</Package>
```

### Using the NuGet Package
In order for a UWP application to use a NuGet package of `DMBridgeComponent`, it
must manually be configured to allow activating the `DMBridgeComponent` classes.
This can be done by manually editing the `Package.appxmanifest` file associated
with the project and adding:

```xml
<Package>
...
  <Extensions>
    <Extension Category="windows.activatableClass.inProcessServer">
      <InProcessServer>
        <Path>DMBridgeComponent.dll</Path>
        <ActivatableClass ActivatableClassId="DMBridgeComponent.ComputerNameBridge" ThreadingModel="both" />
        <ActivatableClass ActivatableClassId="DMBridgeComponent.NTServiceBridge" ThreadingModel="both" />
        <ActivatableClass ActivatableClassId="DMBridgeComponent.TelemetryLevelBridge" ThreadingModel="both" />
      </InProcessServer>
    </Extension>
  </Extensions>
...
</Package>
```

## Thrown Exceptions
DM Bridge APIs return HRESULTs to denote if a method succeeded, or what
exception was thrown. `DMBridgeComponent` will automatically convert these
HRESULTs to a corresponding `Exception` native to the caller's language.

For example, if an API returns `ERROR_ACCESS_DENIED`, in C# this will be thrown
as
[UnauthorizedAccessException](https://msdn.microsoft.com/en-us/library/system.unauthorizedaccessexception(v=vs.110).aspx).

If there are no equivalent native exceptions for an HRESULT, `DMBridgeComponent`
will throw a plain `System.Exception` (or the language's equivalent), and the
HRESULT can be checked using the `HResult` property of it, or the `Message`
property for a user-friendly description of the error.


## API Documentation
- [Computer Name](dm-uwp-api/dm-uwp-api-computername.md)
- [Service Manager](dm-uwp-api/dm-uwp-api-servicemanager.md)
- [Telemetry Level](dm-uwp-api/dm-uwp-api-telemetrylevel.md)
