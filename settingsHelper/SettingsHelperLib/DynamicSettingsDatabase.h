#pragma once

#ifndef S__DYNAMICSETTINGSDATABASE_H
#define S__DYNAMICSETTINGSDATABASE_H

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

#endif // S__DYNAMICSETTINGSDATABASE_H
