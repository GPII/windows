/**
 * IDynamicSettingsDatabase definition.
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

#include <roapi.h>
#include <windows.foundation.h>

/**
 * RESEARCH DATA: This is the RAW pointer table found for the interface IDynamicSettingsDatabase
 * ============================================================================
 *
 * 6 -> SystemSettings::NotificationsDataModel::AppSettingsDynamicDatabase::GetSetting(struct HSTRING__ *,struct SystemSettings::DataModel::ISettingItem * *)
 */

__interface __declspec(uuid("2bea7562-66a9-47a1-aebd-aa188e3a1b57"))
IDynamicSettingsDatabase : public IInspectable {
    HRESULT GetSetting(HSTRING id, ISettingItem** setting);
};
