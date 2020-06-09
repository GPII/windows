/**
 * DynamicSettingDatabase - a wrapper for IDynamicSettingsDatabase.
 *
 * Copyright 2019 Raising the Floor - US
 *
 * Licensed under the New BSD license. You may not use this file except in
 * compliance with this License.
 *
 * You may obtain a copy of the License at
 * https://github.com/GPII/universal/blob/master/LICENSE.txt
 */

#include "stdafx.h"
#include "DynamicSettingsDatabase.h"

#include <utility>

using std::pair;

const vector<pair<wstring, vector<wstring>>> supportedDynamicSettings {
    // SystemSettings.Notifications.QuietMomentsDynamicDatabase
    {
        L"SystemSettings_QuietMoments_On_Scheduled_Mode",
        {
            L"SystemSettings_QuietMoments_Scheduled_Mode_Enabled",
            L"SystemSettings_QuietMoments_Scheduled_Mode_StartTime",
            L"SystemSettings_QuietMoments_Scheduled_Mode_EndTime",
            L"SystemSettings_QuietMoments_Scheduled_Mode_Frequency",
            L"SystemSettings_QuietMoments_Scheduled_Mode_Profile",
            L"SystemSettings_QuietMoments_Scheduled_Mode_ShouldShowNotification"
        },
    },
    {
        L"SystemSettings_QuietMoments_On_Full_Screen_Mode",
        {
            L"SystemSettings_QuietMoments_Full_Screen_Mode_Enabled",
            L"SystemSettings_QuietMoments_Full_Screen_Mode_Profile",
            L"SystemSettings_QuietMoments_Full_Screen_Mode_ShouldShowNotification"
        },
    },
    {
        L"SystemSettings_QuietMoments_On_Game_Mode",
        {
            L"SystemSettings_QuietMoments_Game_Mode_Enabled",
            L"SystemSettings_QuietMoments_Game_Mode_Profile",
            L"SystemSettings_QuietMoments_Game_Mode_ShouldShowNotification"
        }
    },
    {
        L"SystemSettings_QuietMoments_On_Home_Mode",
        {
            L"SystemSettings_QuietMoments_Home_Mode_Enabled",
            L"SystemSettings_QuietMoments_Home_Mode_Profile",
            L"SystemSettings_QuietMoments_Home_Mode_ShouldShowNotification",
            L"SystemSettings_QuietMoments_Home_Mode_ShouldShowNotification",
            L"SystemSettings_QuietMoments_Home_Mode_ChangeAddress"
        }
    },
    {
        L"SystemSettings_QuietMoments_On_Presentation_Mode",
        {
            L"SystemSettings_QuietMoments_Presentation_Mode_Enabled",
            L"SystemSettings_QuietMoments_Presentation_Mode_Profile",
            L"SystemSettings_QuietMoments_Presentation_Mode_ShouldShowNotification"
        }
    },
    {
        L"SystemSettings_QuietMoments_On_Full_Screen_Mode",
        {
            L"SystemSettings_QuietMoments_Full_Screen_Mode_Enabled",
            L"SystemSettings_QuietMoments_Full_Screen_Mode_Profile",
            L"SystemSettings_QuietMoments_Full_Screen_Mode_ShouldShowNotification"
        }
    },
    // SystemSettings.NotificationsDataModel.AppSettingsDynamicDatabase
    {
        // Settings present in every element inside the settings collection
        L"SystemSettings_Notifications_AppList",
        {
            L"SystemSettings_Notifications_AppNotificationKeepContentAboveLockPrivate",
            L"SystemSettings_Notifications_AppNotificationBanners",
            L"SystemSettings_Notifications_AppNotificationLed",
            L"SystemSettings_Notifications_AppNotificationMaxCollapsedGroupItemCountSetting",
            L"SystemSettings_Notifications_TopPriorityCommandSetting",
            L"SystemSettings_Notifications_AppNotificationPrioritizationSetting",
            L"SystemSettings_Notifications_AppShowNotificationsInActionCenter",
            L"SystemSettings_Notifications_AppNotificationSoundToggle",
            L"SystemSettings_Notifications_AppNotifications",
            L"SystemSettings_Notifications_AppNotificationVibrate"
        }
    },
    {
        // TODO: Check 'kind', collection?
        L"SystemSettings_Notifications_QuietHours_Profile",
        {
            L"SystemSettings_Notifications_QuietHoursProfile_AddApp",
            L"SystemSettings_Notifications_QuietHoursProfile_AddPeople",
            L"SystemSettings_Notifications_QuietHoursProfile_AllowedApps",
            L"SystemSettings_Notifications_QuietHoursProfile_AllowedPeople",
            L"SystemSettings_Notifications_QuietHoursProfile_AllowAllCalls_CortanaEnabled",
            L"SystemSettings_Notifications_QuietHoursProfile_AllowAllCalls_CortanaDisabled",
            L"SystemSettings_Notifications_QuietHoursProfile_AllowAllPeople",
            L"SystemSettings_Notifications_QuietHoursProfile_AllowAllReminders",
            L"SystemSettings_Notifications_QuietHoursProfile_AllowAllTexts",
            L"SystemSettings_Notifications_QuietHoursProfile_AllowRepeatCalls",
            L"SystemSettings_Notifications_QuietHoursProfile_Subtitle"
        }
    }
};

DynamicSettingDatabase::DynamicSettingDatabase(wstring dbSettingsName, ATL::CComPtr<IDynamicSettingsDatabase> settingDatabase)
    : _settingDatabase(settingDatabase), dbSettingsName(dbSettingsName) {}


BOOL isSupportedDb(const wstring& settingId) {
    BOOL result { false };

    for (const auto& supportedDb : supportedDynamicSettings) {
        if (supportedDb.first == settingId) {
            result = true;
        }
    }

    return result;
}

HRESULT getSupportedDbSettings(const DynamicSettingDatabase& database, vector<wstring>& settingIds) {
    HRESULT result { ERROR_NOT_SUPPORTED };

    for (const auto& elem : supportedDynamicSettings) {
        if (elem.first == database.dbSettingsName) {
            settingIds = elem.second;
            result = ERROR_SUCCESS;
        }
    }

    return result;
}

HRESULT loadSettingDatabase(const wstring& settingId, SettingItem& settingItem, DynamicSettingDatabase& _dynSettingDatabase) {
    HRESULT res { ERROR_SUCCESS };
    wstring propId { L"DynamicSettingsDatabaseValue" };

    if (isSupportedDb(settingId)) {
        ATL::CComPtr<IDynamicSettingsDatabase> propSettingDatabase = NULL;
        res = settingItem.GetProperty(propId, reinterpret_cast<IInspectable**>(&propSettingDatabase));

        if (res == ERROR_SUCCESS) {
            DynamicSettingDatabase dynSettingDatabase {settingId, propSettingDatabase};
            _dynSettingDatabase = dynSettingDatabase;
        } else if (res == E_NOTIMPL) {
            ATL::CComPtr<IInspectable> iValSettingDatabase = NULL;
            res = settingItem.GetValue(propId, iValSettingDatabase);

            if (res == ERROR_SUCCESS) {
                // Set the IDynamicSettingsDatabase with the queried value
                ATL::CComPtr<IDynamicSettingsDatabase> valSettingDatabase {
                    reinterpret_cast<IDynamicSettingsDatabase*>(iValSettingDatabase.Detach())
                };
                DynamicSettingDatabase dynSettingDatabase { settingId, valSettingDatabase };
                _dynSettingDatabase = dynSettingDatabase;
            }
        } else {
            // There is no other way of accessing the inner database
            // the requested database shouldn't exist
            res = E_INVALIDARG;
        }
    } else {
        res = ERROR_NOT_SUPPORTED;
    }

    return res;
}

HRESULT DynamicSettingDatabase::GetDatabaseSettings(vector<DbSettingItem>& dbSettings) const {
    HRESULT res { ERROR_SUCCESS };
    vector<wstring> dbSettingsIds {};
    vector<DbSettingItem> _dbSettings {};

    res = getSupportedDbSettings(*this, dbSettingsIds);

    if (res == ERROR_SUCCESS) {
        for (const auto& settingId : dbSettingsIds) {
            ATL::CComPtr<ISettingItem> pSettingItem = NULL;
            HSTRING hSettingId = NULL;

            WindowsCreateString(settingId.c_str(), static_cast<UINT32>(settingId.size()), &hSettingId);
            res = this->_settingDatabase->GetSetting(hSettingId, &pSettingItem);

            if (res == ERROR_SUCCESS) {
                DbSettingItem setting { settingId, pSettingItem };
                _dbSettings.push_back(setting);
            }

            WindowsDeleteString(hSettingId);
        }
    }

    if (res == ERROR_SUCCESS) {
        dbSettings = _dbSettings;
    }

    return res;
}

