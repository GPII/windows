# Windows system settings handler helper

This solution exposes Windows 10 settings in a way that can be
accessed using a JSON based API. Examples of this payloads can
be found in:

## Usage

Reads a JSON array of payloads from stdin:

    SettingsHelper < payload.json

## Payload JSON

The payload sent to the application's standard input looks like this:

```json
[
    {
        "settingID": "SystemSettings_Accessibility_Magnifier_IsEnabled",
        "method": "SetValue",
        "parameters": [ true ]
    }
]
```

* `settingID`: The setting ID. See `WindowsSettings -list`
* `method`: The method of  to invoke.
* `parameters`: Parameters to pass (optional).

There can be several items, allowing more than one setting to be applied.

See [examples](./SettingsHelperTests/payloads)

A more detailed explanation of how this payloads should be
supplied to the handler can also be found in the documentation of
the systemSettingsHandler module, located in:

* gpii/node_modules/systemSettingsHandler/src/systemSettingsHandler.js

### Methods

| Method |  |
| --- | --- |
| `GetValue [name]` | Gets a value, returning the result. `name` is used if there are different values. |
| `SetValue [name,] value` | Sets the value. |

## Response

Each payload item yields a response:

```json
    {
        "returnValue": 5,
        "isError": true,
        "errorMessage": "oops"
    }
```

* `returnValue`: The return value of the method (optional).
* `isError`: true if there was an error (optional).
* `errorMessage`: The error message (optional).

## Limitations

* Relies heavily on undocumented behavior.
* The availability of settings depends on the exact Windows version.

## Building

```powershell
$msbuild = Get-MSBuild "4.0"

# Build the settingsHelper solution
nuget restore .\settingsHelper\SettingsHelper.sln

$env:VisualStudioVersion = '15.0'
$settingsHelperDir = Join-Path $rootDir "settingsHelper"
Invoke-Command $msbuild "SettingsHelper.sln /p:Configuration=Release /p:Platform=`"x64`"" $settingsHelperDir
```

## How it works

Magic.
