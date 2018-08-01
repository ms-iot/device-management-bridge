# Telemetry Level API (TelemetryLevelBridge)

## Telemetry Levels
 Value                         | Description
-------------------------------|--------------------------------------------
`0` | [Security](https://docs.microsoft.com/en-us/windows/privacy/configure-windows-diagnostic-data-in-your-organization#security-level)
`1` | [Basic](https://docs.microsoft.com/en-us/windows/privacy/configure-windows-diagnostic-data-in-your-organization#basic-level)
`2` | [Enhanced](https://docs.microsoft.com/en-us/windows/privacy/configure-windows-diagnostic-data-in-your-organization#enhanced-level)
`3` | [Full](https://docs.microsoft.com/en-us/windows/privacy/configure-windows-diagnostic-data-in-your-organization#full-level)

## Summary

 Member                        | Description
-------------------------------|--------------------------------------------
`void`  [`SetLevel`](#setlevel) `([int] level)` | Set the telemetry level.
`int`   [`GetLevel`](#getlevel) `()` | Get the telemetry level.

## Members

### SetLevel

Set the telemetry level.

#### Parameters
- `level` The desired telemetry level. See
[Telemetry Levels](#telemetry-levels) for a list of possible values.

### GetLevel

Get the telemetry level.

### Returns
An integer representing the telemetry level. See
[Telemetry Levels](#telemetry-levels) for a list of possible values.