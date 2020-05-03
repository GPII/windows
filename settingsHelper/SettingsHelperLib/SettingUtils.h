/**
 * Utility functions to handle access settings values.
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

#include "SettingItem.h"

#include <windows.foundation.h>

using namespace ABI::Windows::Foundation;

/// <summary>
///   Returns a vector holding the subkeys of the specified registry key.
/// </summary>
HRESULT getRegSubKeys(const HKEY& hKey, vector<wstring>& rKeys);
/// <summary>
///   Changes a setting value using the supplied string.
/// </summary>
HRESULT setSettingValue(const VARIANT& value, SettingItem& settingItem);
/// <summary>
///   Returns the string representation of the value contained in the IPropertyValue.
/// </summary>
HRESULT toString(const ATL::CComPtr<IPropertyValue>& propValue, wstring& rValueStr);
/// <summary>
///
/// </summary>
vector<wstring> split(const wstring& str, const wstring& delim, const wstring& esc_sec);
/// <summary>
///   Receives the path to a setting and splits it into the secuence of SettingsId
///   that needs to be queried to get to the desired setting.
/// </summary>
/// <param name="settingPath">The complete path to the setting.</param>
/// <param name="settings">The vector with the secuence of settings to be obtained.</param>
HRESULT splitSettingPath(const wstring& settingPath, vector<wstring>& idsPath);
/// <summary>
///   Gets the corresponding DLL to the supplied setting identifier.
/// </summary>
/// <param name="settingId">A string containing the setting id for which the dll needs to be searched.</param>
/// <param name="settingDLL">A reference to a string to be filled with the setting dll location.</param>
/// <returns>
///   An HRESULT error if the operation failed or ERROR_SUCCESS. Possible errors:
///     -ERROR_OPEN_FAILED: If the registry key containing the Id can't be openned.
/// </returns>
HRESULT getSettingDLL(const std::wstring& settingId, std::wstring& settingDLL);

class SettingAPI {
private:
    /// <summary>
    ///  The base library that need to be loaded before any other lib.
    /// </summary>
    HMODULE baseLibrary { NULL };
    /// <summary>
    ///  A list of the already loaded libraries.
    /// </summary>
    vector<pair<wstring, HMODULE>> loadLibraries {};

    /// <summary>
    ///  Checks if a library is already loaded.
    /// </summary>
    /// <param name="libraryPath"></param>
    /// <returns></returns>
    BOOL isLibraryLoaded(wstring libraryPath);
    /// <summary>
    ///  Gets an already loaded library.
    /// </summary>
    /// <param name="libPath">The loaded library path in the filesystem.</param>
    /// <param name="rLib">A reference to be filled with the found loaded library.</param>
    /// <returns>
    ///  An error code specifying ERROR_SUCCESS if the operation was successful or
    ///  one of the following error codes:
    ///     - ERROR_NOT_FOUND: If the library isn't already loaded.
    /// </returns>
    HRESULT getLoadedLibrary(const wstring& libPath, HMODULE& rLib);
    /// <summary>
    ///  Loads the specified library and register it in the 'loadedLibraries' map.
    /// </summary>
    /// <param name="libPath">The path to the dll binary to be loaded.</param>
    /// <returns>
    ///  An error code specifying ERROR_SUCCESS if the operation was successful or
    ///  one of the following error codes:
    /// </returns>
    HRESULT loadLibrary(const wstring& libPath, HMODULE& rHLib);
    /// <summary>
    ///  Loads the library associated with a particular setting Id.
    /// </summary>
    HRESULT loadSettingLibrary(const wstring& settingId, HMODULE& lib);

public:
    /// <summary>
    ///  Empty constructor.
    /// </summary>
    SettingAPI();
    /// <summary>
    ///  Desctructor, it deinitialize loaded libraries in the proper order.
    /// </summary>
    /// ~SettingAPI();
    /// <summary>
    ///  Copy constructor operator.
    /// </summary>
    /// <param name="other"></param>
    /// <returns></returns>
    SettingAPI& operator=(SettingAPI& other) = delete;
    /// <summary>
    ///   Loads the base setting exposed through the DLL.
    /// </summary>
    /// <param name="settingId">The setting id to be loaded.</param>
    /// <param name="baseSetting">A reference to the SettingItem to be filled.</param>
    /// <returns>
    ///   An HRESULT error if the operation failed or ERROR_SUCCESS. Possible errors:
    ///     - ERROR_OPEN_FAILED: If the inner GetSettingDLL operation fails.
    ///     - E_OUTOFMEMORY: If the system runs out of memory and WindowsCreateString fails.
    ///     - ERROR_MOD_NOT_FOUND: If the LoadLibrary function fails.
    /// </returns>
    HRESULT loadBaseSetting(const std::wstring& settingId, SettingItem& settingItem);
    /// <summary>
    ///  Gets the settings inside a collection.
    /// </summary>
    /// <param name="collSetting"></param>
    /// <param name="rSettings"></param>
    /// <returns></returns>
    HRESULT getCollectionSettings(SettingItem& collSetting, vector<SettingItem>& rSettings);
    /// <summary>
    ///  Load the settingsi
    /// </summary>
    /// <param name="ids"></param>
    /// <param name="settingCollection"></param>
    /// <param name="rSettings"></param>
    /// <returns></returns>
    HRESULT getCollectionSettings(const vector<wstring>& ids, SettingItem& collSetting, vector<SettingItem>& rSettings);
    /// <summary>
    ///
    /// </summary>
    /// <param name="id"></param>
    /// <param name="settingCollection"></param>
    /// <param name="rSetting"></param>
    /// <returns></returns>
    HRESULT getCollectionSetting(const wstring& id, SettingItem& settingCollection, SettingItem& rSetting);

    /// <summary>
    ///  Initializes the SettingAPI.
    /// </summary>
    friend SettingAPI& LoadSettingAPI(HRESULT& rErrCode);
    /// <summary>
    ///  Deinitializes the SettingAPI.
    /// </summary>
    /// <returns></returns>
    friend HRESULT UnloadSettingsAPI(SettingAPI& rSAPI);
};
