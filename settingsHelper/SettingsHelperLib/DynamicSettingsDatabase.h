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

#pragma once

#include "IDynamicSettingsDatabase.h"
#include "SettingItem.h"

#include <atlbase.h>
#include <string>
#include <vector>

using std::wstring;
using std::vector;

struct DynamicSettingDatabase {
private:
    ATL::CComPtr<IDynamicSettingsDatabase> _settingDatabase;

public:
    wstring dbSettingsName {};

    DynamicSettingDatabase() {}
    DynamicSettingDatabase(wstring dbSettingsName, ATL::CComPtr<IDynamicSettingsDatabase> settingDatabase);

    HRESULT GetDatabaseSettings(vector<DbSettingItem>& settings) const;
};

BOOL isSupportedDb(const wstring& settingId);
HRESULT loadSettingDatabase(const wstring& settingId, SettingItem& settingItem, DynamicSettingDatabase& dynSettingDatabase);
HRESULT getSupportedDbSettings(const DynamicSettingDatabase& database, vector<wstring>& settingIds);
