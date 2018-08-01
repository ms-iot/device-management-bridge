# Configuring DM Bridge

- [Command Line](#command-line)
    - [Log File Path](#log-file-path)
        - [Enable](#enable-file-logging)
        - [Disable](#disable-file-logging)
        - [Check Status](#check-file-logging-status)
    - [Configuration File Path](#configuration-file-path)
        - [Set Custom Path](#set-configuration-file-path)
        - [Restore Default Path](#restore-configuration-file-path)
        - [Check Path](#check-configuration-file-path)
- [Configuration File](#configuration-file)
    - [Enabling API](#enabling-api)
    - [Configuring API](#configuring-api)
        - [ServiceManager](#servicemanager)
    - [Example](#example)

## Command Line
Both DM Bridge's log file and setting file can be configured via the command
line.

### Log File Path
By default, `DMBridge.exe` will not log to a file, instead using only
[ETW Tracing](https://docs.microsoft.com/en-us/dotnet/framework/wcf/samples/etw-tracing).

#### Enable File Logging
To enable file logging, run the following command as Administrator:

> DMBridge.exe -logging enable {filepath}

where `{filepath}` is the full file path of the desired log file. For example,
to log to `C:\dm.log`, run:

> DMBridge.exe -logging enable C:\dm.log

#### Disable File Logging
To disable file logging, run the following command as Administrator:

> DMBridge.exe -logging disable

#### Check File Logging Status
To see if file logging is enabled, and if so where to, run:

> DMBridge.exe -logging state

### Configuration File Path
By default, `DMBridge.exe` will look for the file `dmbridge.config.json` in the
same directory as the executable.

#### Set Configuration File Path
To set a custom configuration file path, run the following command as
Administrator:

> DMBridge.exe -config set {filepath}

where `{filepath}` is the full file path of the desired configuration file.
For example, to use `C:\config.json`, run:

> DMBridge.exe -config set C:\config.json


#### Restore Configuration File Path
To restore the configuration file path to default, run the following command as
Administrator:

> DMBridge.exe -config default

#### Check Configuration File Path
To see where the current configuration file is set to, run:

> DMBridge.exe -config state

## Configuration File
To allow DM Bridge to be easily customized without needing to recompile the
solution, there is support for a JSON configuration file.

If the configuration file is not present or has syntax errors then default
values are used.

**NOTE**: All configurations are optional. However, if a configuration is
specified it will override the default value.

### Enabling API

By default, DM Bridge will load and listen for all API. However, if an `api`
array is defined in the configuration file, it will only load the specified API.
If another application tries to call an API that was not loaded, the call will
return `RPC_S_UNKNOWN_IF`. API names are case-insensitive.

#### API List
 Configuration String                         | API
-------------------------------|--------------------------------------------
`computername` | [Computer Name](dm-uwp-api/dm-uwp-api-computername.md)
`servicemanager` | [Service Manager](dm-uwp-api/dm-uwp-api-servicemanager.md)
`telemetry` | [Telemetry Level](dm-uwp-api/dm-uwp-api-telemetrylevel.md)

Example
```json
{
    "api": [
        "computername",
        "telemetry"
    ]
}

```

### Configurable API

#### ServiceManager

ServiceManager has an array of whitelisted services that can be stopped/started.
By default, only the `w32time` service is on the whitelist. The whitelist is
case-insensitive and does not affect the ability to query a service.

Example:
```json
{
    "servicemanager": {
        "whitelist": [
            "w32time",
            "service1",
            "service2"
        ]
    }
}

```

### Example
The below configuration would only enable the `servicemanager` and `telemetry`
APIs, and would define a custom service whitelist.

```json
{
    "api": [
        "servicemanager",
        "telemetry"
    ],
    "servicemanager": {
        "whitelist": [
            "dhcp",
            "w32time"
        ]
    }
}
```