# ServiceManager API (NTServiceBridge)

## Summary

 Member                        | Description
-------------------------------|--------------------------------------------
`void`  [`Start`](#start)   `([string] serviceName)` | Start the specified service. Only services present on a pre-defined whitelist can be manually started.
`void`  [`Stop`](#stop) `([string] serviceName)` | Stop the specified service. Only services present on a pre-defined whitelist can be manually stopped.
`int`   [`Query`](#query)   `([string] serviceName)` | Query the state of aspecific service. Non-whitelisted services can be queried.

## Members

### Start

Start the specified service. Only services present on a pre-defined whitelist
can be manually started.

#### Parameters
- `serviceName` The service name to start, it is case insensitive. Has a max
length of 256 characters, and cannot include the forward or backward slash.

#### Exceptions
- `ERROR_ACCESS_DENIED` Thrown if the requested service is not whitelisted.
- `ERROR_INVALID_SERVICENAME` Thrown if serviceName is an invalid length or
has illegal characters.

### Stop

Stop the specified service. Only services present on a pre-defined whitelist can
be manually stopped.

#### Parameters
- `serviceName` The service name to stop, it is case insensitive. Has a max
length of 256 characters, and cannot include the forward or backward slash.

#### Exceptions
- `ERROR_ACCESS_DENIED` Thrown if the requested service is not whitelisted.
- `ERROR_INVALID_SERVICENAME` Thrown if serviceName is an invalid length or has
illegal characters.

### Query

Query the state of a specific service. Non-whitelisted services can be queried.

#### Parameters
- `serviceName` The service name to query, it is case insensitive. Has a max
length of 256 characters, and cannot include the forward or backward slash.

#### Exceptions
- `ERROR_INVALID_SERVICENAME` Thrown if serviceName is an invalid length or has
illegal characters.

### Returns
An integer representing the service state. See
https://docs.microsoft.com/en-us/windows/desktop/api/winsvc/ns-winsvc-_service_status
for the possible values.