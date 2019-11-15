#pragma once

#ifndef I__IDynamicSettingsDatabase
#define I__IDynamicSettingsDatabase

#include "ISettingItem.h"

#include <roapi.h>
#include <windows.foundation.h>

// SystemSettings::NotificationsDataModel::AppSettingsDynamicDatabase::GetSetting(struct HSTRING__ *,struct SystemSettings::DataModel::ISettingItem * *)

__interface __declspec(uuid("2bea7562-66a9-47a1-aebd-aa188e3a1b57"))
IDynamicSettingsDatabase : public IInspectable {
    HRESULT GetSetting(HSTRING id, ISettingItem** setting);
};

#endif // !I__IDynamicSettingsDatabase
