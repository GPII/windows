/**
 * Definition of ISettingItem interface.
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

#include <roapi.h>
#include <windows.foundation.h>
#include <windows.ui.core.h>

/**
 * RESEARCH DATA: This is the RAW pointer table found for the interface ISettingItem
 * ============================================================================
 *
 * 6  -> SystemSettings::DataModel::CSettingBase::get_Id(struct HSTRING__ * *)
 * 7  -> SettingsHandlers_Notifications.dll!SystemSettings::DataModel::CSettingBase::get_Type(enum SystemSettings::DataModel::SettingType *)
 * 9  -> SettingsHandlers_Notifications.dll!SystemSettings::DataModel::CSettingBase::get_IsEnabled(unsigned char *)
 * 13 -> SystemSettings::DataModel::CDataSettingBase<long(*)(struct SystemSettings::DataModel::SettingDBItem const *, struct SystemSettings::DataModel::ISettingItem * *)>::GetValue(struct HSTRING__ *, struct IInspectable * *)
 * 14 -> SystemSettings::NotificationsDataModel::CustomizableQuickActions::SetValue(struct HSTRING__ *, struct IInspectable *)
 * 15 -> SystemSettings::DataModel::CSettingBase::GetProperty(struct HSTRING__ *, struct IInspectable * *)
 * 16 -> SystemSettings::DataModel::CSettingBase::SetProperty(struct HSTRING__ *, struct IInspectable *)
 * 17 -> SystemSettings::DataModel::CSettingBase::Invoke(struct Windows::UI::Core::ICoreWindow *,struct Windows::Foundation::Rect)
 * 18 -> SystemSettings::DataModel::CSettingBase::add_SettingChanged(struct Windows::Foundation::ITypedEventHandler<struct IInspectable *, struct HSTRING__ *> *, struct EventRegistrationToken *)
 * 19 -> SystemSettings::DataModel::CSettingBase::remove_SettingChanged(struct EventRegistrationToken)
 * 20 -> SystemSettings::DataModel::CGetValueAsyncHelper<class SystemSettings::DataModel::CInBoxATBoolStateSetting>::`vector deleting destructor'(unsigned int)
 * 21 -> SystemSettings::DataModel::CSettingBase::SetIsUpdating(unsigned char)
 */

/// <summary>
///  Represents different kind of settings.
/// </summary>
enum class SettingType {
    Empty = -1,
    Custom = 0,

    // Read-only
    DisplayString = 1,
    LabeledString = 2,

    // Values (use GetValue/SetValue)
    Boolean = 3,
    Range = 4,
    String = 5,
    List = 6,

    // Perform the setting action
    Action = 7,

    // Is a ISettingCollection
    SettingCollection = 8
};

/// <summary>
///  Raw interface representing a Setting, should not be directly accessed.
/// </summary>
__interface __declspec(uuid("40C037CC-D8BF-489E-8697-D66BAA3221BF"))
ISettingItem : public IInspectable {
public:
    int get_Id(HSTRING* id);
    int get_SettingType(SettingType* val);
    int get_IsSetByGroupPolicy(BOOL* val);
    int get_IsEnabled(BOOL* val);
    int get_IsApplicable(BOOL* val);
    int get_Description(HSTRING* desc);

    int get_IsUpdating(BOOL* val);
    int GetValue(HSTRING__* name, IInspectable** item);
    HRESULT SetValue(HSTRING__* name, IInspectable* item);

    int GetProperty(HSTRING__*, IInspectable**);
    int SetProperty(HSTRING__*, IInspectable*);

    int Invoke(ABI::Windows::UI::Core::ICoreWindow* wnd, IInspectable* rect);
    int add_SettingChanged(ABI::Windows::Foundation::ITypedEventHandler<IInspectable *, HSTRING__ *>* eventHnd, EventRegistrationToken* token);
    int remove_SettingChanged(EventRegistrationToken token);
};
