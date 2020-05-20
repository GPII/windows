/**
 * Datatypes representing payload and functions.
 *
 * Copyright 2019 Raising the Floor - US
 *
 * Licensed under the New BSD license. You may not use this file except in
 * compliance with this License.
 *
 * You may obtain a copy of the License at
 * https://github.com/GPII/universal/blob/master/LICENSE.txt
 */

#pragma once

#include "stdafx.h"
#include "SettingItem.h"

#include <windows.foundation.h>
#include <atlbase.h>

#include <string>
#include <vector>
#include <utility>

using std::wstring;
using std::vector;
using std::pair;

using namespace ABI::Windows::Foundation;

/// <summary>
///  Structure holding the values required to perform a particular action over
///  a setting. Parameters can represent either a JSON object, which holds a pair
///  <'Identifier', 'Value'>, or a simple value.
/// </summary>
struct Parameter {
    /// <summary>
    ///  A pair <'Identifier', 'Value'> used when the parameter is of kind 'Object'.
    /// </summary>
    pair<wstring, ATL::CComPtr<IPropertyValue>> oIdVal;
    /// <summary>
    ///  A IPropertyValue representing a value when the Parameter isn't of kind
    ///  'Object'.
    /// </summary>
    ATL::CComPtr<IPropertyValue> iPropVal;
    /// <summary>
    ///  Property to identify how the parameter was constructed as an 'Object'.
    /// </summary>
    bool isObject { false };
    /// <summary>
    ///  Property to identify if the parameter was empty constructed.
    /// </summary>
    bool isEmpty { true };

    /// <summary>
    ///  Empty constructor, used to represent a empty parameter.
    /// </summary>
    Parameter();
    /// <summary>
    ///  Constructor for constructing the parameter as an object.
    /// </summary>
    /// <param name="_objIdVal">
    ///  A pair with which the Parameter is going to initialized.
    /// </param>
    Parameter(pair<wstring, ATL::CComPtr<IPropertyValue>> _objIdVal);
    /// <summary>
    ///  Constructor for constructing the parameter as a value.
    /// </summary>
    /// <param name="_iPropVal">
    ///  A smart pointer to an IPropertyValue  with which the Parameter is going to initialized.
    /// </param>
    Parameter(ATL::CComPtr<IPropertyValue> _iPropVal);
};

/// <summary>
///  Action that is going to be performed over a setting.
/// </summary>
struct Action {
    /// <summary>
    /// The setting id in which the action needs to be performed.
    /// </summary>
    wstring settingID;
    /// <summary>
    /// The setting method to be called.
    /// </summary>
    wstring method;
    /// <summary>
    /// The parameters to be passed to the method that is going to be called.
    /// </summary>
    vector<Parameter> params;
};

/// <summary>
///  The result of each 'Action' requested to the application.
/// </summary>
struct Result {
    /// <summary>
    ///  The id of the setting that was the target of the action.
    /// </summary>
    wstring settingID;
    /// <summary>
    ///  Flag identifying if the operation was a success or not.
    /// </summary>
    BOOL isError;
    /// <summary>
    ///  Human readable message describing the problem encountered
    ///  during the operation. Empty in case of success.
    /// </summary>
    wstring errorMessage;
    /// <summary>
    ///  The value that is returned as a result of the operation.
    /// </summary>
    wstring returnValue;
};

/// <summary>
/// Parse the input payload and returns a secuence of actions to be applied.
/// </summary>
/// <param name="payload">The payload to be parsed.</param>
/// <param name="actions">The sequence of actions to be filled with the payload.</param>
/// <returns>
///   An HRESULT error if the operation failed or ERROR_SUCCESS. Possible errors:
///     - WEB_E_INVALID_JSON_STRING: If the JSON from the payload is invalid.
///     - WEB_E_JSON_VALUE_NOT_FOUND: If one of the required JSON fields isn't present in the payload.
///     - E_OUTOFMEMORY: If the system runs out of memory and WindowsCreateString fails.
/// </returns>
HRESULT parsePayload(const wstring & payload, vector<pair<Action, HRESULT>>& actions);
/// <summary>
/// Serialize the result of the operation to communicate it back to the caller.
/// </summary>
/// <param name="result">The result of the operation.</param>
/// <param name="str">The string to be filled with the result serialization.</param>
/// <returns>
///  An HRESULT error if the operation failed or ERROR_SUCCESS. Possible errors:
///     - E_OUTOFMEMORY: If the system runs out of memory.
/// </returns>
HRESULT serializeResult(const Result& result, std::wstring& str);
