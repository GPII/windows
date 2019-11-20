/**
 * Payload processing functions.
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

#include <Windows.h>
#include <windows.foundation.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

#include "SettingUtils.h"
#include "IPropertyValueUtils.h"
#include "SettingItemEventHandler.h"
#include "StringConversion.h"
#include "Payload.h"

using std::wstring;
using std::vector;
using std::pair;

#pragma comment (lib, "WindowsApp.lib")

/// <summary>
///  Applies one setting action into a setting.
/// </summary>
/// <param name="valueId">The value identifier, in case of being a inner setting, this value is a setting id.</param>
/// <param name="action">The action to be performed over the setting.</param>
/// <param name="setting">The setting that is going to receive the action.</param>
/// <param name="rVal">A string containing the result of the operation.</param>
/// <returns>
///   ERROR_SUCCESS in case of success or one of the following error codes:
///     - E_NOTIMPL:
///         + If the operation is not supported on the supplied setting.
///         + If the supplied IPropertyValue within the action can't be compared
///           with the contained IPropertyValue in the target setting.
///     - E_INVALIDARG:
///         + If the supplied 'valueId' isn't supported.
///         + If the supplied IPropertyValue withing the action can't be converted into a string.
/// </returns>
HRESULT handleSettingAction(const wstring& valueId, const Action& action, SettingItem& setting, wstring& rVal);
/// <summary>
///  Serializes a vector of pairs of setting '<id, value>'.
/// </summary>
/// <param name="settingsValues">
///  The settings values to be serialized.
/// </param>
/// <returns>
///  The string containing the serialization of the supplied
///  setting values.
/// </returns>
wstring serializeReturnValues(vector<pair<wstring, wstring>> settingsValues);
/// <summary>
///  Handle an action over a SettingCollection.
/// </summary>
/// <param name="lib">Reference to the already loaded settings library.</param>
/// <param name="valueId">The value id target by the action.</param>
/// <param name="action">The action to be performed over the value id.</param>
/// <param name="setting">Reference to the setting that is going to receive the action.</param>
/// <param name="rResult">
///  Reference to 'Result' to be filled with the outcome of the action.
/// </param>
/// <returns>
///  ERROR_SUCCESS in case of success or one of the following error codes:
///     - 'handleSettingAction' error code if one of actions over the elements of the
///       collection fails.
///     - E_INVALIDARG:
///         + If the supplied setting isn't of type SettingsCollection.
///         + If obtaining the settings from the settings collection fails.
/// </returns>
HRESULT handleCollectionAction(
    SettingAPI&     sAPI,
    const wstring&  valueId,
    const Action&   action,
    SettingItem&    setting,
    Result&         rResult
);

/// <summary>
///  Type alias for the base setting identifier.
/// </summary>
using BaseSettingId = wstring;
/// <summary>
///  Type alias for the inner setting identifier.
/// </summary>
using InnerSettingId = wstring;
/// <summary>
///  Type alias for the SettingPath.
/// </summary>
using SettingPath = pair<BaseSettingId, InnerSettingId>;

/// <summary>
///  Get the setting path holded in the Action request.
/// </summary>
/// <param name="action">The action that holds the setting id.</param>
/// <param name="rPath">The path to the setting to be filled.</param>
/// <returns>
///  ERROR_SUCCESS in case of success or E_INVALIDARG if the supplied setting id
///  can't be splitted into a valid SettingPath.
/// </returns>
HRESULT getSettingPath(const Action& action, SettingPath& rPath);
/// <summary>
///	 Handles a action over a setting of collection kind.
/// </summary>
/// <param name="lib">Reference to the already loaded settings library.</param>
/// <param name="action">An action to be performed.</param>
/// <param name="rResult">
///  The result of applying the action.
/// </param>
/// <returns>
///  ERROR_SUCCESS in case of success or one of the following error codes:
///     - E_INVALIDARG if the setting isn't supported or failed to load.
///     - 'handleSettingAction' error code if the action supplied isn't of
///       SettingCollection type.
///     - 'handleCollectinoAction' error code if the action supplied is of
///       SettingCollection type.
/// </returns>
HRESULT handleAction(SettingAPI& sAPI, Action action, Result& rResult);
/// <summary>
///  Read the data in the input stream.
///
///  NOTE: This function is blocking, and should be called from
///  it's own thread.
/// </summary>
/// <param name="payloadStr">
///  A pointer to a wstring to be filled with the contents of
///  the standard input.
/// </param>
void readInputStream(wstring* payloadStr);
/// <summary>
///  Creates an ouput string from a vector of results.
/// </summary>
/// <param name="results">
///  A with the Result of operations.
/// </param>
/// <returns>
///  A ouput string with the proper serialization of the supplied
///  results.
/// </returns>
wstring buildOutputStr(const vector<Result>& results);
/// <summary>
///  Creates an error message with the supplied error code. The error code
///  will appear in HEX format in the message, so no information about it is
///  lost.
/// </summary>
/// <param name="errCode">The error code that should appear in the message.</param>
/// <returns>An error message.</returns>
wstring invalidPayloadMsg(HRESULT errCode);
/// <summary>
///  Get the input payload for the application, if the file switch is specified,
///  the input payload is get from the file specified in the command line input.
///  In other case, the input is taking from the standard input, if the input
///  isn't received within 1 second of the function call, no input is assumed.
/// </summary>
/// <param name="pInput">
///  The program input encapsulated into a pointer to a pair.
/// </param>
/// <param name="rPayloadStr">
///  A reference to a string to be filled with the input payload.
/// </param>
/// <returns>
///   ERROR_SUCCESS if everything went fine, otherwise one of this errors is returned:
///     -
/// </returns>
HRESULT getInputPayload(pair<int, wchar_t**>* pInput, wstring& rPayloadStr);
/// <summary>
///  Handle the complete input payload from the program and return a result.
/// </summary>
/// <param name="pInput">
///  Pointer to the program payload composed of two pointer
/// </param>
/// <returns>
///  ERROR_SUCCESS in case of success or one of the following error codes:
/// </returns>
HRESULT handlePayload(pair<int, wchar_t**>* pInput);
