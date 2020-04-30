/**
 * Definition of ISettingsCollection interface.
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

/**
 * RESEARCH DATA: This is the RAW pointer table found for the interface ISettingsCollection
 * ============================================================================
 *
 * 22 -> SystemSettings::DataModel::CSettingBase::GetInitializationResult(void)
 * 23 -> SystemSettings::DataModel::CDataSettingBase<long(*)(struct SystemSettings::DataModel::SettingDBItem const *, struct SystemSettings::DataModel::ISettingItem * *)>::DoGenericAsyncWork(void)
 * 24 -> SystemSettings::DataModel::CGenericAsyncHelper<class SystemSettings::DataModel::CSettingBase>::StartGenericAsyncWork(class SystemSettings::DataModel::CSettingBase *)
 * 25 -> SystemSettings::DataModel::CGenericAsyncHelper<class SystemSettings::DataModel::CSettingBase>::SetSkipConcurrentOperations(bool)
 * 26 -> SystemSettings::NotificationsDataModel::NotificationsAppList::GetValue(struct IInspectable * *)
 * 27 -> SystemSettings::DataModel::CDataSettingBase<long (*)(struct SystemSettings::DataModel::SettingDBItem const *,struct SystemSettings::DataModel::ISettingItem * *)>::SetValue(bool)
 * 28 -> SystemSettings::DataModel::CDataSettingBase<long(*)(struct SystemSettings::DataModel::SettingDBItem const *, struct SystemSettings::DataModel::ISettingItem * *)>::SetValue(struct HSTRING__ *)
 * 29 -> SystemSettings::DataModel::CDataSettingBase<long (*)(struct SystemSettings::DataModel::SettingDBItem const *,struct SystemSettings::DataModel::ISettingItem * *)>::SetValue(struct Windows::Foundation::IPropertyValue *)
 * 30 -> SystemSettings::NotificationsDataModel::NotificationsAppList::GetNamedValue(struct HSTRING__ *,struct IInspectable * *)
 * 31 -> SystemSettings::DataModel::CDataSettingBase<long (*)(struct SystemSettings::DataModel::SettingDBItem const *,struct SystemSettings::DataModel::ISettingItem * *)>::SetNullValue(void)
 * 32 -> SystemSettings::DataModel::CDataSettingBase<long (*)(struct SystemSettings::DataModel::SettingDBItem const *,struct SystemSettings::DataModel::ISettingItem * *)>::SetNullValueForUser(struct HSTRING__ *)
 * 33 -> SystemSettings::NotificationsDataModel::NotificationsAppList::GetSettingsCollection(void)
 * 34 -> SystemSettings::NotificationsDataModel::NotificationsAppList::DoGetValueAsyncWork(void)
 * 35 -> SystemSettings::DataModel::CGetValueAsyncHelper<class SystemSettings::DataModel::CGenericAsyncHelper<class SystemSettings::DataModel::CBoolSetting> >::StartGetValueAsyncWork(class SystemSettings::DataModel::CSettingBase *)
 * 36 -> SystemSettings::DataModel::CGetValueAsyncHelper<class SystemSettings::DataModel::CCollectionSetting>::StartGetValueAsyncWork(unsigned short const *,class SystemSettings::DataModel::CSettingBase *)
 * 37 -> SystemSettings::DataModel::CGetValueAsyncHelper<class SystemSettings::DataModel::CActionSetting>::ResetValueQueried(void)
 */

/// <summary>
///  This is the RAW interface obtained representing the settings collections,
///  this interface should not be accessed directly.
/// </summary>
__interface __declspec(uuid("62948522-8857-4C63-9B85-8FFF20128F6A"))
ISettingsCollection : ISettingItem {
    int GetInitializationResult(void);
    int GetInitializationResult1(void);
    int DoGenericAsyncWork(void);
    // Unknown -- Private method
    int StartGenericAsyncWork(IInspectable*);
    // =======

    int SetSkipConcurrentOperations(bool);
    int GetValue(IInspectable** item);
    int SetValue(bool);
    int SetValue(HSTRING);
    int SetValue(ABI::Windows::Foundation::IPropertyValue*);
    int SetNullValue(void);
    int SetNullValueForUser(HSTRING);

    // Unknown
    IInspectable* GetSettingsCollection(void);
    IInspectable* GetSettingsCollection2(void);
    // =======

    int DoGetValueAsyncWork(void);
    int DoGetValueAsyncWork2(void);
    int DoGetValueAsyncWork3(void);
    // Unknown -- Private method
    int StartGetValueAsyncWork(IInspectable*);
    int StartGetValueAsyncWork(unsigned short const *, IInspectable*);
    // =======

    int ResetValueQueried(void);
};