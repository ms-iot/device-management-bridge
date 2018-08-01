# Adding New APIs

- [Step-by-step Guide](#step-by-step-guide)
    - [DMBridgeRpcInterface](#dmbridgerpcinterface)
    - [DMBridge](#dmbridge)
    - [DMBridgeComponent](#dmbridgecomponent)
- [Adding a Configurable API](#adding-a-configurable-api)
- [Consuming the new API via NuGet](#consuming-the-new-api-via-nuget)
## Step-by-step Guide

Adding a new set of APIs to DM Bridge involves 3 main changes:
- Define a new set of Rpc methods in `DMBridgeRpcInterface` that `DMBridge` will
serve.
- Add the Rpc server implementation of those methods in `DMBridge`.
- Add API wrappers in `DMBridgeComponent` for UWP applications.

### DMBridgeRpcInterface

This project defines the Rpc methods that `DMBridgeComponent` will use to call
`DMBridge`. The following steps detail how to add a new set of Rpc methods.

- Add a new interface file
    - Right click on `DMBridgeRpcInterface -> API` and press
`Add -> New Item...`
    - In the `Add New Item` dialog, select `Visual C++ -> Code` and choose
`Midl File (.idl)`
    - Name the file `{APIName}Interface.idl`. For example, to create a `Foobar`
API, the file would be named `FoobarInterface.idl`
    - Since the RpcInterface outputs a single set of files that aggregates all
individual `idl` files, the new idl file will need to be excluded from building
individually. To do so, right click on the new `idl` file and press `Properties`
. In the opened dialog, under `Configuration Properties -> General`, set
`Excluded From Build` to `Yes`.
- Define the new interface
    - When defining the new `idl` file, be sure to generate a new unique GUID
    - Ensure that all methods defined have a return type of `HRESULT`, this is
needed to simplify exception handling. If you need to return another value, use
an `[out]` parameter.
    - Postfix all methods with `Rpc` to make following a stacktrace simpler when
debugging.
    - For an example, see
[NTServiceInterface.idl](../src/DMBridgeInterface/NTServiceInterface.idl)
    - For more help, see the
[Defining an Interface with MIDL](https://docs.microsoft.com/en-us/windows/desktop/Rpc/using-midl#defining-an-interface-with-midl)
documentation.
- Include the new interface in the aggregation file
    - Open `DMBridgeInterface.idl`, and add `#include "{IDLName}"` on a new line
at the end of the file. For example, if the `idl` made before was called
`FoobarInterface.idl`, add `#include "FoobarInterface.idl"` to the file.
- Rebuild `DMBridgeRpcInterface`

### DMBridge
This project is the actual service that performs the API calls for other
applications. It can be run via `DMBridgeComponent`, or any other Rpc client.
With the new API interface defined in `DMBridgeRpcInterface`, implementations
of the new methods will need to be created. To do so, follow the below steps.
- Make DMBridge aware of the new interface.
    - Open `DMBridge -> Source Files -> Config -> DMBridgeConfig.cpp` and add a
new entry to the `interfaceMap` declaration in the
`DMBridgeConfig::MakeInterfaceMap()` method. This variable maps a friendly
name for the new API to the actual Rpc interface. The friendly name can be used
to enable or disable it via a [configuration file](dm-configuration.md) at
runtime. The `RPC_IF_HANDLE` of the new interface will be called
`{InterfaceName}_{version}_s_ifspec` For example, if the interface made before
was called `Foobar`, and had `version(1.0)` set in the `idl` file, it would be
called `Foobar_v1_0_s_ifspec`.
- Add a header file for the implementation
    - Right click on `DMBridge -> Header Files -> API` and press
`Add -> New Item...`.
    - In the `Add New Item` dialog, select `Visual C++ -> Code` and choose
`Header File (.h)`.
    - Name the file `{APIName}.h`. For example, to create an API `Foobar`, the
file would be named `Foobar.h`.
    - Define a class that will provide implementations for the new API.
    - See [ComputerName.h](../src/DMBridge/ComputerName.h) for an example.
- Add the implementation for the new API
    - Right on `DMBridge -> Source Files -> API` and press `Add -> New Item...`.
    - In the `Add New Item` dialog, select `Visual C++ -> Code` and choose
`C++ File (.cpp)`.
    - Name the file `{APIName}.cpp`. For example, to create an API `Foobar`, the
file would be named `Foobar.cpp`.
    - In the newly created file, create the method implementations for your API.
    - Add wrapper methods that match the method signatures from the Rpc `idl`
file, and call the correlating class implementation. This will ensure the class
methods are called when the Rpc method is invoked.
    - Any methods should catch all exceptions they may throw, and convert them
to a corresponding `HRESULT` and return that value. It is the responsibility of
the Rpc client to handle the `HRESULT` and throw any exceptions on the client
side. To convert a `WIN32` error code to an `HRESULT` use
[HRESULT_FROM_WIN32](https://docs.microsoft.com/en-us/windows/desktop/api/winerror/nf-winerror-hresult_from_win32).
    - See [ComputerName.cpp](../src/DMBridge/ComputerName.cpp) for an example
- Rebuild `DMBridge`

### DMBridgeComponent
This project is the WinRT component that is called by UWP applications and
methods that are defined here can be used by various languages. It communicates
to `DMBridge` via an Rpc client. To add the new set of APIs to it, follow the
below steps.
- Create the interface file
    - Right click on `DMBridgeComponent -> API` and press `Add -> New Item...`
    - In the `Add New Item` dialog, select `Visual C++` and choose
`Data Model (C++/WinRT)`.
    - Name the file `{APIName}Bridge.idl`. For example, to create a `Foobar` API
, the file would be named `FoobarBridge.idl`
    - When defining the new `idl`, be sure to generate a new, unique, GUID.
    - Ensure that all methods defined have a method return type of `HRESULT`,
this is needed to simplify exception handling. The `HRESULT` will be handled
internally and not exposed to the API caller. To define the return type of the
API, use `[out, retval]`.
    - For an example, see
[ComputerNameBridge.idl](../src/DMBridgeComponent/ComputerNameBridge.idl).
    - For more help, see the
[C++/WinRT](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/)
documentation.
- Update the associated `.cpp` and `.h`
    - Expand the newly created `idl` file to reveal `{APIName}Bridge.cpp` and
`{APIName}Bridge.h`.
    - Update both files to reflect your new methods. See
[ComputerNameBridge.cpp](../src/DMBridgeComponent/ComputerNameBridge.cpp) and
[ComputerNameBridge.h](../src/DMBridgeComponent/ComputerNameBridge.h) for an
example.
    - The `cpp` file should act as a simple wrapper around the Rpc endpoints
that `DMBridge` serves. Error handling and input validation should be done in
`DMBridge`. In general, you should be able to simply have the method's body,
outside of any preprocessing of arguments, be:
`winrt::check_hresult(RpcNormalize(::{RpcMethodName}, this->hRpcBinding,{RpcMethod Arg1, Arg2, ..., ArgN}));`.
    - The `RpcNormalize` calls wraps the Rpc method wraps the call in a special
try/catch that normalizes all return codes, even ones thrown by Rpc itself, to
be an `HRESULT`, which `winrt::check_hresult` maps to the correct response.
This call will automatically map common HRESULT values to their corresponding
exception type.
- Rebuild `DMBridgeComponent`

## Adding a Configurable API

Individual APIs can be configured using a JSON file, as described in the
[DM Bridge Configuration](dm-configuration.md) documentation. To add support
for runtime configurations to the new API, you will need to define a new
`{APIName}Config` class with a header and implementation file for it in
the `DMBridge` project under `[Header/Source] Files -> Config -> API`. See 
[NTServiceConfig.h](../src/DMBridge/NTServiceConfig.h) and
[NTServiceConfig.cpp](../src/DMBridge/NTServiceConfig.cpp) for an example.

This class should derive from [`IConfig`](../src/DMBridge/IConfig.h) and
provide implementations for `ParseJSON` and `ApplyDefaults`. The actual
API implementation files should also be aware of this new configuration class
and reference it. See [NTService.h](../src/DMBridge/NTService.h) and
[NTService.cpp](../src/DMBridge/NTService.cpp) for an example.

Finally, you will need to make `DMBridge.exe` deserialize the configuration file
into your new config class on startup. Simply add a new step in the
`LoadConfiguration` method in [DMBridge.cpp](../src/DMBridge/DMBridge.cpp). See
the `NTService::ApplyConfig` step in that function for an example which ensures
the service does not crash if there are parsing issues.


## Consuming the new API via NuGet

As described in the [DM Bridge UWP API](dm-uwp-api.md) documentation, if
`DMBridgeComponent` is referenced via NuGet, classes will have to be manually
set as activatable to be used. This can be done by manually editing the
`Package.appxmanifest` file associated with the project and adding:

```xml
<Package>
...
  <Extensions>
    <Extension Category="windows.activatableClass.inProcessServer">
      <InProcessServer>
        <Path>DMBridgeComponent.dll</Path>
        ...
        <ActivatableClass ActivatableClassId="DMBridgeComponent.{APIClass}" ThreadingModel="both" />
        ...
      </InProcessServer>
    </Extension>
  </Extensions>
...
</Package>
```

where `{APIClass}` is the name of the runtimeclass defined in the interface
for the new API.