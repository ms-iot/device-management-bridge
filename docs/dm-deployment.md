# Deploying DM Bridge

- [Service](#service)
    - [Where to Place the DMBridge Executable](#where-to-place-the-dmbridge-executable)
    - [Installing the Service](#installing-the-service)
        - [Enable Automatic Start](#enable-automatic-start)
    - [Removing the Service](#removing-the-service)
- [Console Application (for testing)](#console-application)

## Service

### Where to Place the DMBridge Executable

When `DMBridge.exe` is registered as a service, the service will point to
the program's path at the time it was registered. It is recommend to place
`DMBridge.exe` either on the OS or the DATA partition of a Windows IoT Core
device.

The choice between the OS and the DATA partition depends on if the
`DMBridge.exe` service should persist across factory resets. Placing it on the
DATA partition means the service will removed during a reset, where placing it
on the OS partition will make it persist.

### Installing the Service

To configure `DMBridge.exe` as a registered service, run the following command
as Administrator:

> DMBridge.exe -install

This command will register a service named `SystemConfiguratorBridge` that
runs as `DMBridge.exe` as `SYSTEM`. The service will not be set to automatically
start on system boot.

**NOTE**: If you wish to move the binary after it has
been registered, re-run the command on the binary in the new location.

#### Enable Automatic Start

To configure the registered `SystemConfiguratorBridge` service to start
automatically when the system boots, run the following command as Administrator:

> sc.exe config SystemConfiguratorBridge start=auto

### Removing the Service

To remove `DMBridge.exe` as a registered service, run the following command
as Administrator:

> DMBridge.exe -uninstall

This command will also stop the service, if it is running, before removing it.
## Console Application

Running `DMBridge.exe` as a console application provides an easy way to debug
the application, as logs will be printed to the console window. With the
exception of testing, it is recommend to
[configure `DMBridge.exe` as a service](#service) for production use.

To run `DMBridge.exe` as a regular console application, run the following
command as Administrator:

> DMBridge.exe -console

To exit the application, press `Ctr-C`.