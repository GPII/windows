/**
 * BaseSettingItem - a wrapper for ISettingItem.
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

#include "ISettingItem.h"

#include <atlbase.h>
#include <windows.foundation.h>
#include <windows.foundation.collections.h>

#include <string>
#include <vector>
#include <utility>

using std::wstring;
using std::vector;
using std::pair;

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;

struct BaseSettingItem {
protected:
    UINT timeout = 50;
    UINT retries = 5;

public:

    /// <summary>
    ///  The id of the setting being hold.
    /// </summary>
    wstring settingId {};
    /// <summary>
    ///  Pointer to the inner setting.
    /// </summary>
    ATL::CComPtr<ISettingItem> setting { NULL };

    /// <summary>
    ///  Empty constructor.
    /// </summary>
    BaseSettingItem();
    /// <summary>
    ///  Constructs the BaseSettingItem using a pointer to IBaseSettingItem.
    /// </summary>
    /// <param name="setting"></param>
    BaseSettingItem(wstring settingId, ATL::CComPtr<ISettingItem> BaseSettingItem);
    /// <summary>
    ///  Copy constructor.
    /// </summary>
    /// <param name="other">The setting item to be copied.</param>
    BaseSettingItem(const BaseSettingItem& other);
    /// <summary>
    ///  Copy assignment operator.
    /// </summary>
    BaseSettingItem& operator=(const BaseSettingItem& other);
    /// <summary>
    ///  Copy constructor.
    /// </summary>
    /// <param name="other">The setting item to be copied.</param>
    /// <summary>
    ///  Destructor that frees the encapsulated IBaseSettingItem.
    /// </summary>
    ///  ~BaseSettingItem();
    /// <summary>
    ///  Gets the IBaseSettingItem identifier.
    /// </summary>
    /// <param name="id">A reference to a wstring to be filled with the current setting Id.</param>
    /// <returns>An HRESULT error if the operation failed or ERROR_SUCCESS.</returns>
    UINT GetId(std::wstring& id) const;
    /// <summary>
    ///  Gets the current setting Type.
    /// </summary>
    /// <param name="val">A pointer to a SettingType to be filled with the current type.</param>
    /// <returns>An HRESULT error if the operation failed or ERROR_SUCCESS.</returns>
    UINT GetSettingType(SettingType* val) const;
    /// <summary>
    /// Gets if the setting is set by a group policy.
    /// </summary>
    /// <param name="val">A pointer ot a bool to be filled with the current value.</param>
    /// <returns>An HRESULT error if the operation failed or ERROR_SUCCESS.</returns>
    UINT GetIsSetByGroupPolicy(BOOL* val) const;
    /// <summary>
    /// Gets if the setting is set by a group policy.
    /// </summary>
    /// <param name="val">A pointer ot a bool to be filled with the current value.</param>
    /// <returns>An HRESULT error if the operation failed or ERROR_SUCCESS.</returns>
    UINT GetIsEnabled(BOOL* val) const;
    /// <summary>
    /// Gets if the settings is applicable.
    /// </summary>
    /// <param name="val">A pointer ot a bool to be filled with the current value.</param>
    /// <returns>An HRESULT error if the operation failed or ERROR_SUCCESS.</returns>
    UINT GetIsApplicable(BOOL* val) const;
    /// <summary>
    ///  Gets a description of the setting.
    ///
    ///  Note: This value can be null, if not, many time this value is used to
    ///  present a description of the setting in the UI.
    /// </summary>
    /// <param name="desc">A reference to a wstring to be filled with the current setting description.</param>
    /// <returns>An HRESULT error if the operation failed or ERROR_SUCCESS.</returns>
    UINT GetDescription(std::wstring& desc) const;
    /// <summary>
    ///  Gets if the current setting value is being updated, if not, it means that
    ///  it already holds the correct value, and it's ready for a 'get operation'.
    /// </summary>
    /// <param name="val">A pointer ot a bool to be filled with the current value.</param>
    /// <returns>An HRESULT error if the operation failed or ERROR_SUCCESS.</returns>
    UINT GetIsUpdating(BOOL* val) const;
    /// <summary>
    ///  Gets the value of the current stored setting that matches with the supplied
    ///  identifier.
    /// </summary>
    /// <param name="id">The identifier of the setting to be retrieved, most of the
    ///  times this just "Value".</param>
    /// <param name="item"> A pointer to a IInspectable* that will hold the current value, if the
    ///  operation doesn't succeed, "item" will be set to NULL. </param>
    /// <returns>
    ///   An HRESULT error if the operation failed or ERROR_SUCCESS. Possible errors:
    ///     - E_NOTIMPL: The setting doesn't have implemented the method; this can be
    ///       caused because the setting doesn't support this method, and doesn't contains
    ///       any value.
    /// </returns>
    UINT GetValue(wstring id, ATL::CComPtr<IInspectable>& item);
    /// <summary>
    ///  Sets the current value for the setting that matches the supplied identifier.
    /// </summary>
    /// <param name="id">The identifier of the setting to be set, most of the times this
    ///  just "Value".</param>
    /// <param name="item">A pointer to a IInspecable that holds the current value to be set,
    ///  most of the times this should be a IPropertyValue.</param>
    /// <returns>
    ///   An HRESULT error if the operation failed or ERROR_SUCCESS. Possible errors:
    ///     - TYPE_E_TYPEMISMATCH: The IPropertyValue supplied withing the 'item' parameter
    ///       is not the proper type for the setting.
    ///     - E_NOTIMPL: The setting doesn't have implemented the method; this can be
    ///       caused because the setting doesn't support this method, and doesn't contains
    ///       any value.
    /// </returns>
    HRESULT SetValue(const wstring& id, ATL::CComPtr<IPropertyValue>& value);
    /// <summary>
    ///  Gets the value of the current stored property that matches with the supplied
    ///  identifier.
    /// </summary>
    /// <param name="id">The identifier of the setting to be retrieved, most of the times this is
    ///  just "Value".</param>
    /// <param name="item">A pointer to a IInspectable* that will hold the current value, if the
    ///  operation doesn't succeed, "item" will be set to NULL.</param>
    /// <returns>An HRESULT error if the operation failed or ERROR_SUCCESS.</returns>
    UINT GetProperty(wstring id, IInspectable** item) const;
    /// <summary>
    ///  Sets the current value for the property that matches the supplied identifier.
    /// </summary>
    /// <param name="id">The identifier of the setting to be set, most of the times this is
    ///  just "Value".</param>
    /// <param name="item">A pointer to a IInspecable that holds the current value to be set,
    ///  most of the times this should be a IPropertyValue.</param>
    /// <returns>An HRESULT error if the operation failed or ERROR_SUCCESS.</returns>
    UINT SetProperty(wstring id, IInspectable* item);
    /// <summary>
    ///  In case of the setting being of "Action" kind, executes the action associated
    ///  with it.
    ///
    ///  NOTE: Sometimes this action will have no effect, since it sometimes requires
    ///  a pointer to the ICoreWindow of the application that will display the action.
    /// </summary>
    /// <returns>An HRESULT error if the operation failed or ERROR_SUCCESS.</returns>
    UINT Invoke();
};
