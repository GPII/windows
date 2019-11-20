/**
 * Represents a Setting - ISettingItem wrapper.
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

#include "BaseSettingItem.h"
#include "DbSettingItem.h"

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

struct SettingItem : BaseSettingItem {
private:
    /// <summary>
    ///  Maximum number of times the delay for Getting/Setting
    ///  a setting value is waited.
    /// </summary>
    UINT maxIt { 10 };
    /// <summary>
    ///  Lazy vector with DBSettingItems supported by the
    ///  setting. This vector will be filled the first time this
    ///  settings are requested.
    /// </summary>
    vector<DbSettingItem> dbSettings {};
    /// <summary>
    ///  This id specifies the parent setting of the current setting.
    ///  This id is specially useful in situations where the setting
    ///  is a setting from a Collection, in this case, we want
    ///  to specify this Id as a way to know if we support accesing
    ///  the inner db settings of this particular setting.
    /// </summary>
    wstring parentId {};
    /// <summary>
    ///  Private helper method encapsulating the waiting logic
    ///  necessary for properly set a new setting a new value.
    /// </summary>
    HRESULT _SetValue(DbSettingItem& dbSetting, ATL::CComPtr<IPropertyValue>& item);

public:
    using BaseSettingItem::BaseSettingItem;

    /// <summary>
    ///  Resources that needs to be loaded for the setting to be accessed.
    /// </summary>
    vector<SettingItem> assocSettings {};

    /// <summary>
    /// Default constructor
    /// </summary>
    SettingItem();

    SettingItem(wstring parentId, wstring settingId, ATL::CComPtr<ISettingItem> settingItem);
    /// <summary>
    ///  Constructs a SettingItem with the loaded required settings that it needs to work.
    /// </summary>
    /// <param name="settingId"></param>
    /// <param name="settingItem"></param>
    /// <param name="assocSettings"></param>
    SettingItem(wstring settingId, ATL::CComPtr<ISettingItem> settingItem, vector<SettingItem> assocSettings);
    /// <summary>
    ///  Copy constructor.
    /// </summary>
    /// <param name="other">The setting item to be copied.</param>
    SettingItem(const SettingItem& other);
    /// <summary>
    ///  Copy assignment operator.
    /// </summary>
    SettingItem& operator=(const SettingItem& other);

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
};
