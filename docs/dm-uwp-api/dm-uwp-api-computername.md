# ComputerName API (ComputerNameBridge)

## Summary

 Member                        | Description
-------------------------------|--------------------------------------------
`void`  [`SetName`](#setname)   `([string] computerName)` | Set the local computer name. Changes will take effect on next reboot.
`string`    [`GetName`](#getname)   `()` | Retrieve the NetBIOS name of the computer.
`bool`  [`IsRenamePending`](#isrenamepending)   `()` | Return true if a computer rename is pending.

## Members

### SetName

Set the local computer name. Changes will take effect on next reboot.

This function updates both the DNS Hostname and NetBIOS.

#### Parameters
- `computerName` The new computer name. Has a max length of 15 characters, can
only include alphanumeric characters, and cannot contain only digits.

#### Exceptions
- `ERROR_INVALID_COMPUTERNAME` Thrown if computerName is an invalid length or
has illegal characters.

### GetName

Retrieve the NetBIOS name of the computer.

#### Returns
The local computer name.


### IsRenamePending

Return true if a computer rename is pending.

#### Returns
True if a computer rename is pending