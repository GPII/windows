# hello

Small utility that sets Windows settings via the back-end used by the Windows 10 setting app.

## Usage

Reads a JSON array of payloads from stdin:

    WindowsSettings < payload.json

List all possible settings for the current system:

    WindowsSettings -list


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

See [examples](./examples)


### Methods

| Method |  |
| --- | --- |
| `GetValue [name]` | Gets a value, returning the result. `name` is used if there are different values. |
| `SetValue [name,] value` | Sets the value. |
| `Invoke` | Invokes an `Action` setting. |
| `GetPossibleValues` | Returns a list of settings allowed for `List` settings |
| `IsEnabled` | Returns a boolean indicating if the setting is enabled. |
| `IsApplicable` | Returns a boolean indicating if the setting is applicable. |

These are the exposed methods of the [SettingItem](WindowsSettings/SettingItem.cs) class.


### Setting Types and methods

The type of a setting can be retrieved with `WindowsSettings -list`.


#### `Boolean`, `String`, `List`, `LabeledString`, and `Range`

The `GetValue` method returns the value, and `SetValue` sets it. 

* `Boolean` and `String` settings are straightforward.
* `List` settings accept one of several possible values (or the index, depending on the setting), returned by `GetPossibleValues`.
* `Range` appears to be numeric, determining the min and max value is currently unknown.
* `LabeledString` is read-only.


#### `Action`

Settings of this type perform an action when the `Invoke` method is used.


#### `Custom`, `DisplayString` and `SettingCollection`

Unsupported - needs further investigation.


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

* Relies heavily on undocumented behaviour.
* Various settings are applied asynchronously, with no apparent completion callback.
* Some settings crash.
* The availability of settings depends on the exact Windows version.


## Building

    msbuild WindowsSettings.sln /t:Rebuild /p:Configuration=Release /p:Platform="Any CPU"


## How it works

Magic.
