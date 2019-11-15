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

    HRESULT res = ERROR_SUCCESS;
    BOOL isUpdating = false;

    HSTRING hId = NULL;
    res = WindowsCreateString(id.c_str(), static_cast<UINT32>(id.size()), &hId);

    IInspectable* curValue = NULL;
    if (res == ERROR_SUCCESS) {
        if (id == L"Value" || id == L"DynamicSettingsDatabaseValue") {
            res = BaseSettingItem::GetValue(id, item);
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
                if (this->dbSettings.empty()) {
                    DynamicSettingDatabase dynSettingDb {};
                    res = loadSettingDatabase(settingId, *this, dynSettingDb);

                    if (res == ERROR_SUCCESS) {
                        res = dynSettingDb.GetDatabaseSettings(this->dbSettings);
                    }
                }

                if (res == ERROR_SUCCESS) {
                    for (auto& setting : this->dbSettings) {
                        if (id == setting.settingId) {
                            res = setting.GetValue(L"Value", item);
                            break;
                        }
                    }
                }
            } else {
                // TODO: Improve error code
                res = E_INVALIDARG;
            }
        }

    }

    return res;
}

HRESULT SettingItem::SetValue(const wstring& id, ATL::CComPtr<IPropertyValue>& item) {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };

    HRESULT res { ERROR_SUCCESS };

    if (id == L"Value") {
        BOOL completed = false;
        BOOL isUpdating { true };
        UINT maxIt { 10 };

        ATL::CComPtr<ITypedEventHandler<IInspectable*, HSTRING>> handler =
            new ITypedEventHandler<IInspectable*, HSTRING>(&completed);
        EventRegistrationToken token { 0 };

        res = this->setting->add_SettingChanged(handler, &token);

        if (res == ERROR_SUCCESS) {
            res = BaseSettingItem::SetValue(id, item);

            if (res == ERROR_SUCCESS) {
                UINT it = 0;
                while (completed != TRUE && isUpdating == TRUE) {
                    if (it < maxIt) {
                        this->setting->get_IsUpdating(&isUpdating);
                        System::Threading::Thread::Sleep(100);

                        it++;
                    } else {
                        completed = true;
                        res = ERROR_TIMEOUT;
                    }
                }
            }
        }

        this->setting->remove_SettingChanged(token);
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
            if (this->dbSettings.empty()) {
                DynamicSettingDatabase dynSettingDb {};
                res = loadSettingDatabase(settingId, *this, dynSettingDb);

                if (res == ERROR_SUCCESS) {
                    res = dynSettingDb.GetDatabaseSettings(this->dbSettings);
                }
            }

            if (res == ERROR_SUCCESS) {
                for (auto& setting : this->dbSettings) {
                    if (id == setting.settingId) {
                        BOOL isUpdating { true };
                        BOOL completed { false };
                        BOOL innerUpdating { true };
                        UINT maxIt { 10 };

                        ATL::CComPtr<ITypedEventHandler<IInspectable*, HSTRING>> handler =
                            new ITypedEventHandler<IInspectable*, HSTRING>(&completed);
                        EventRegistrationToken token { 0 };

                        res = this->setting->add_SettingChanged(handler, &token);

                        res = setting.SetValue(L"Value", item);

                        if (res == ERROR_SUCCESS) {
                            UINT it = 0;
                            while (completed != TRUE || isUpdating == TRUE || innerUpdating == TRUE) {
                                if (it < maxIt) {
                                    this->setting->get_IsUpdating(&isUpdating);
                                    setting.GetIsUpdating(&innerUpdating);

                                    System::Threading::Thread::Sleep(100);

                                    it++;
                                } else {
                                    completed = true;
                                    res = ERROR_TIMEOUT;
                                }
                            }
                        }

                        res = this->setting->remove_SettingChanged(token);

                        break;
                    }
                }
            }
        } else {
            // TODO: Improve error code
            res = E_INVALIDARG;
        }
    }

    return res;
}
