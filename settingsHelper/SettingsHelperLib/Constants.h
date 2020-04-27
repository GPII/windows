/**
 * Constants
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

#include <string>
#include <vector>
#include <map>

#include <basetsd.h>

using std::wstring;
using std::vector;
using std::map;

namespace constants {
    /// <summary>
    ///  Maximum length for a registry key name.
    /// </summary>
    const static UINT32 MAX_KEY_LENGTH { 255 };
    /// <summary>
    ///  Maximum length for a registry key value.
    /// </summary>
    const static UINT32 MAX_VALUE_NAME { 16383 };
    /// <summary>
    ///  Base registry key that holds all the settings ids.
    /// </summary>
    const wstring& BaseRegPath();
    /// <summary>
    ///  Path to the base library that needs to be loaded in order to get the API proper
    ///  functionality.
    /// </summary>
    const wstring& BaseLibPath();
    /// <summary>
    ///  List of known settings that doesn't work in the target platform.
    /// </summary>
    const vector<wstring>& KnownFaultySettings();
    /// <summary>
    ///  List of known libs that present problems for being loaded.
    /// </summary>
    const vector<wstring>& KnownFaultyLibs();
    /// <summary>
    ///  This libraries are dependent and should be loaded toguether in order for
    ///  proper operation and later unloading.
    /// </summary>
    const map<wstring, vector<wstring>>& CoupledLibs();
}
