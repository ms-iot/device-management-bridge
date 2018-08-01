# Building DM Bridge

Once the prerequisites are met, the `DMBridge` solution can simply be built like
any other project/solution in Visual Studio.

**NOTE**: If you encounter build errors after changing target architecture,
perform `Rebuild Solution`.

- [Prerequisites](#prerequisites)
  - [Compiling Requirements](#compiling-requirements)
  - [NuGet Requirements](#nuget-requirements)
  - [Runtime Requirements](#runtime-requirements)
- [Building a NuGet Package](#building-a-nuget-package)

### Prerequisites
#### Compiling Requirements
- Visual Studio 2017 ([download](https://www.visualstudio.com/downloads)).
  - Make sure the following is selected:
    - VC++ 2017 v141 toolset (x86, x64)
    - Visual C++ compilers and libraries for ARM.
    - Visual C++ 2017 redistributable update.
    - Visual C++ runtime for UWP.
    - Visual Studio C++ Core Features
    - Visual C++ ATL for x86 and x64
    - Visual C++ ATL for ARM
    - Windows 10 SDK (10.0.15063.0) for Desktop
    - Windows 10 SDK (10.0.15063.0) for UWP
    - Windows 10 SDK (10.0.16299.0) for UWP
  - Install the [C++/WinRT extension](https://marketplace.visualstudio.com/items?itemName=CppWinRTTeam.cppwinrt101804264)

#### NuGet Requirements

To build a NuGet package for `DMBridgeComponent` both `nuget.exe` and
`msbuild.exe` must be available through your `PATH` variable. That is, you must
be able to run `nuget.exe` and `msbuild.exe` from an open terminal without
having to specify the directory it is in.

For Visual Studio 2017, `msbuild` can be usually found in , where `{edition}` is
the edition of VS Studio, e.g. `Enterprise`:

> `%programfiles(x86)%\Microsoft Visual Studio\2017\{edition}\MSBuild\15.0\Bin`

A copy of `nuget.exe` can be obtained from https://www.nuget.org/downloads.

#### Runtime Requirements

By default, the `systemManagement` capability for UWP apps will only have effect
on embedded editions of Windows, such as Windows IoT Core. To be able to run and
test DM Bridge via UWP apps on a desktop installation,
[enable embedded mode](https://docs.microsoft.com/en-us/windows/iot-core/develop-your-app/embeddedmode).


### Building a NuGet Package

Open a terminal and change the current directory to be in repository's `nuget`
folder. Once there, run:

> `PackDMBridgeComponent.cmd {version} {Release/Debug} {rebuild}`

The script will produce a `.nupkg` in the current directory.

For example, to rebuild the solution for all architectures in Release mode, and
make a NuGet package for version `1.0.14` the command would be:

> `PackDMBridgeComponent.cmd 1.0.14 Release rebuild`

To use an existing build, simply omit the `rebuild` argument.

**Note**: omitting `rebuild` requires that `DMBridgeComponent` has been built
for `x86`, `x64`, and `ARM` for the desired configuration (Release/Debug).