@echo off
@REM Examples:
@REM   PackDMBridgeComponent.cmd 1.4.4 Release rebuild
@REM   PackDMBridgeComponent.cmd 1.4.4 Debug

@if "%1"=="" goto MissingParameters
@if "%2"=="" goto MissingParameters
@if "%3"=="rebuild" goto Build
goto Pack

:Build
@REM Build the interface manually as it cannot be directly set as a dependency by the Universal dll (DMBridgeComponent)
SET interfaceProj="%~dp0..\src\DMBridgeInterface\DMBridgeInterface.vcxproj"
SET componentProj="%~dp0..\src\DMBridgeComponent\DMBridgeComponent.vcxproj"

msbuild %interfaceProj% /t:Clean,Build /p:configuration=%2 /p:platform=Win32 || goto End
msbuild %componentProj% /t:Clean,Build /p:configuration=%2 /p:platform=Win32 || goto End

msbuild %interfaceProj% /t:Clean,Build /p:configuration=%2 /p:platform=x64 || goto End
msbuild %componentProj% /t:Clean,Build /p:configuration=%2 /p:platform=x64 || goto End

msbuild %interfaceProj% /t:Clean,Build /p:configuration=%2 /p:platform=ARM || goto End
msbuild %componentProj% /t:Clean,Build /p:configuration=%2 /p:platform=ARM || goto End

:Pack
NuGet.exe pack "%~dp0DMBridgeComponent.nuspec" -Prop Version=%1 -Prop Flavor=%2
goto End

:MissingParameters
@echo.
@echo Usage:
@echo     PackDMBridgeComponent.cmd version flavor rebuild
@echo.
@echo where:
@echo     version: the version of the nuget package to be genered in the form: major.minor.revision.
@echo     flavor : Debug or Release
@echo     rebuild: rebuild or leave empty
@echo.
@echo Example:
@echo     PackDMBridgeComponent.cmd 1.4.4 Release
@echo     PackDMBridgeComponent.cmd 1.4.4 Release rebuild
@echo.

:End