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

#include "stdafx.h"
#include "SettingItem.h"
#include "ISettingsCollection.h"
#include "SettingItemEventHandler.h"
#include "DynamicSettingsDatabase.h"

#include <memory>
#include <atlbase.h>
#include <CoreWindow.h>
#include <windows.foundation.h>

#include <iostream>
#include <sstream>
#include <utility>
#include <string>

#pragma comment (lib, "WindowsApp.lib")

using namespace ATL;

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::UI::Core;

using std::wstring;
using std::istringstream;
using std::pair;

SettingItem::SettingItem() : BaseSettingItem::BaseSettingItem() {};

SettingItem::SettingItem(
    wstring settingId, ATL::CComPtr<ISettingItem> settingItem, vector<SettingItem> assocSettings
) : BaseSettingItem::BaseSettingItem(settingId, settingItem), assocSettings(assocSettings) {}

SettingItem::SettingItem(
    wstring parentId, wstring settingId, ATL::CComPtr<ISettingItem> settingItem
) : BaseSettingItem::BaseSettingItem(settingId, settingItem), parentId(parentId) {}

SettingItem::SettingItem(const SettingItem & other) {
    this->parentId = other.parentId;
    this->setting = other.setting;
    this->settingId = other.settingId;
    this->assocSettings = other.assocSettings;
}

SettingItem& SettingItem::operator=(const SettingItem& other) {
    this->parentId = other.parentId;
    this->setting = other.setting;
    this->settingId = other.settingId;
    this->assocSettings = other.assocSettings;

    return *this;
}

UINT SettingItem::GetValue(wstring id, ATL::CComPtr<IInspectable>& item) {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };
    if (id.empty()) { return E_INVALIDARG; };

    HRESULT errCode { ERROR_SUCCESS };
    BOOL isUpdating { false };
    HSTRING hId { NULL };
    ATL::CComPtr<IInspectable> _item { NULL };

    errCode = WindowsCreateString(id.c_str(), static_cast<UINT32>(id.size()), &hId);
    if (errCode != ERROR_SUCCESS) { goto cleanup; }

    if (id == L"Value" || id == L"DynamicSettingsDatabaseValue") {
        errCode = BaseSettingItem::GetValue(id, _item);

        if (errCode == ERROR_SUCCESS) {
            item = _item;
        }
    } else {
        // Access one of the inner Settings inside the
        // DynamicSettingDatabase hold inside the setting.
        wstring settingId {};
        if (isSupportedDb(this->parentId)) {
            settingId = this->parentId;
        } else {
            settingId = this->settingId;
        }

        if (isSupportedDb(settingId)) {
            if (this->dbSettings.empty()) {
                DynamicSettingDatabase dynSettingDb {};
                errCode = loadSettingDatabase(settingId, *this, dynSettingDb);

                if (errCode == ERROR_SUCCESS) {
                    errCode = dynSettingDb.GetDatabaseSettings(this->dbSettings);
                }
            }

            if (errCode == ERROR_SUCCESS) {
                for (auto& setting : this->dbSettings) {
                    if (id == setting.settingId) {
                        errCode = setting.GetValue(L"Value", _item);
                        break;
                    }
                }
            }

            // The required DynamicDatabaseSetting hasn't been found
            if (_item == NULL) {
                errCode = E_INVALIDARG;
            } else {
                item = _item;
            }
        } else {
            // TODO: Improve error code
            errCode = E_INVALIDARG;
        }
    }

cleanup:
    if (hId != NULL) { WindowsDeleteString(hId); }

    return errCode;
}

HRESULT SettingItem::_SetValue(DbSettingItem& dbSetting, ATL::CComPtr<IPropertyValue>& item) {
    HRESULT errCode { ERROR_SUCCESS };
    BOOL completed { false };
    BOOL isUpdating { true };
    BOOL innerUpdating { false };
    BOOL setInnerSetting { dbSetting.setting != NULL };

    ATL::CComPtr<ITypedEventHandler<IInspectable*, HSTRING>> handler =
        new ITypedEventHandler<IInspectable*, HSTRING>(&completed);
    EventRegistrationToken token { 0 };

    if (setInnerSetting) {
        innerUpdating = true;
    }

    errCode = this->setting->add_SettingChanged(handler, &token);

    if (errCode == ERROR_SUCCESS) {
        if (setInnerSetting) {
            errCode = dbSetting.SetValue(L"Value", item);
        } else {
            errCode = BaseSettingItem::SetValue(L"Value", item);
        }

        if (errCode == ERROR_SUCCESS) {
            UINT it = 0;
            while (completed != TRUE && isUpdating || innerUpdating) {
                if (it < this->maxIt) {
                    this->setting->get_IsUpdating(&isUpdating);
                    if (setInnerSetting) {
                        dbSetting.GetIsUpdating(&innerUpdating);
                    }

                    System::Threading::Thread::Sleep(100);
                    it++;
                } else {
                    completed = true;
                    errCode = ERROR_TIMEOUT;
                }
            }
        }
    }

    this->setting->remove_SettingChanged(token);

    return errCode;
}

HRESULT SettingItem::SetValue(const wstring& id, ATL::CComPtr<IPropertyValue>& item) {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };

    HRESULT errCode { ERROR_SUCCESS };

    if (id == L"Value") {
        errCode = SettingItem::_SetValue(DbSettingItem {}, item);
    } else {
        // Access one of the inner Settings inside the DynamicSettingDatabase
        // holded inside the setting.
        wstring settingId {};
        if (isSupportedDb(this->parentId)) {
            settingId = this->parentId;
        } else {
            settingId = this->settingId;
        }

        if (isSupportedDb(settingId)) {
            BOOL applied { false };

            if (this->dbSettings.empty()) {
                DynamicSettingDatabase dynSettingDb {};
                errCode = loadSettingDatabase(settingId, *this, dynSettingDb);

                if (errCode == ERROR_SUCCESS) {
                    errCode = dynSettingDb.GetDatabaseSettings(this->dbSettings);
                }
            }

            if (errCode == ERROR_SUCCESS) {
                for (auto& setting : this->dbSettings) {
                    if (id == setting.settingId) {
                        errCode = SettingItem::_SetValue(setting, item);
                        applied = TRUE;
                        break;
                    }
                }
            }

            // The required DynamicDatabaseSetting hasn't been found
            if (applied == FALSE) {
                errCode = E_INVALIDARG;
            }
        } else {
            // TODO: Improve error code
            errCode = E_INVALIDARG;
        }
    }

    return errCode;
}
