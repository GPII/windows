/**
 * BaseSettingItem - a wrapper for ISettingItem.
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
#include "BaseSettingItem.h"
#include "ISettingsCollection.h"
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

BaseSettingItem::BaseSettingItem() {}

BaseSettingItem::BaseSettingItem(wstring settingId, ATL::CComPtr<ISettingItem> BaseSettingItem) {
    this->settingId = settingId;
    this->setting = BaseSettingItem;
}

BaseSettingItem::BaseSettingItem(const BaseSettingItem & other) {
    this->setting = other.setting;
    this->settingId = other.settingId;
}

BaseSettingItem& BaseSettingItem::operator=(const BaseSettingItem& other) {
    this->setting = other.setting;
    this->settingId = other.settingId;

    return *this;
}

UINT BaseSettingItem::GetId(std::wstring & id) const {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };

    HSTRING hId = NULL;
    HRESULT res = this->setting->get_Id(&hId);

    if (res == ERROR_SUCCESS && hId != NULL) {
        UINT hIdLenght = 0;
        PCWSTR rawStrBuf = WindowsGetStringRawBuffer(hId, &hIdLenght);

        if (rawStrBuf != NULL) {
            id = std::wstring(rawStrBuf);
        }
    }

    // Release resources
    WindowsDeleteString(hId);

    return res;
}

UINT BaseSettingItem::GetSettingType(SettingType * val) const {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };

    return this->setting->get_SettingType(val);
}

UINT BaseSettingItem::GetIsSetByGroupPolicy(BOOL* val) const {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };

    return this->setting->get_IsSetByGroupPolicy(val);
}

UINT BaseSettingItem::GetIsEnabled(BOOL* val) const {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };

    return this->setting->get_IsEnabled(val);
}

UINT BaseSettingItem::GetIsApplicable(BOOL* val) const {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };

    return this->setting->get_IsApplicable(val);
}

UINT BaseSettingItem::GetDescription(std::wstring& desc) const {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };

    HSTRING hDesc = NULL;
    HRESULT res = this->setting->get_Description(&hDesc);

    if (res == ERROR_SUCCESS && hDesc != NULL) {
        UINT hIdLenght = 0;
        PCWSTR rawStrBuf = WindowsGetStringRawBuffer(hDesc, &hIdLenght);

        if (rawStrBuf != NULL) {
            desc = std::wstring(rawStrBuf);
        }
    }

    // Release resources
    WindowsDeleteString(hDesc);

    return res;
}

UINT BaseSettingItem::GetIsUpdating(BOOL* val) const {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };

    return this->setting->get_IsUpdating(val);
}

UINT BaseSettingItem::GetValue(wstring id, ATL::CComPtr<IInspectable>& item) {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };
    if (id.empty()) { return E_INVALIDARG; };

    HRESULT res = ERROR_SUCCESS;
    BOOL isUpdating = false;

    HSTRING hId = NULL;
    res = WindowsCreateString(id.c_str(), static_cast<UINT32>(id.size()), &hId);

    IInspectable* curValue = NULL;
    if (res == ERROR_SUCCESS) {
        // Access the simple value from the setting
        res = this->setting->GetValue(hId, &curValue);

        if (res == ERROR_SUCCESS) {
            res = this->setting->get_IsUpdating(&isUpdating);

            while (isUpdating == TRUE && res == ERROR_SUCCESS) {
                System::Threading::Thread::Sleep(10);
                res = this->setting->get_IsUpdating(&isUpdating);

                if (res != ERROR_SUCCESS) {
                    break;
                }
            }

            HSTRING otherStr = NULL;
            res = WindowsCreateString(id.c_str(), static_cast<UINT32>(id.size()), &otherStr);

            curValue->Release();

            // TODO: This second get is necessary for some settings, like
            // "Collection" settings. For this ones the second get guarantees
            // that the real value is the one received.
            res = this->setting->GetValue(otherStr, &curValue);
            if (res == ERROR_SUCCESS) {
                item.Attach(curValue);
            } else {
                curValue->Release();
            }
        }
    }

    return res;
}

HRESULT BaseSettingItem::SetValue(const wstring& id, ATL::CComPtr<IPropertyValue>& item) {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };

    HRESULT res { ERROR_SUCCESS };
    HSTRING hId { NULL };
    res = WindowsCreateString(id.c_str(), static_cast<UINT32>(id.size()), &hId);

    if (res == ERROR_SUCCESS) {
        res = this->setting->SetValue(hId, static_cast<IInspectable*>(item));
    }

    WindowsDeleteString(hId);

    return res;
}

UINT BaseSettingItem::GetProperty(wstring id, IInspectable** value) const {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };

    HRESULT res = ERROR_SUCCESS;
    HSTRING hId = NULL;
    res = WindowsCreateString(id.c_str(), static_cast<UINT32>(id.size()), &hId);

    if (res == ERROR_SUCCESS) {
        res = this->setting->GetProperty(hId, value);
    }

    return res;
}

UINT BaseSettingItem::SetProperty(wstring id, IInspectable* value) {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };

    HRESULT res = ERROR_SUCCESS;
    HSTRING hId = NULL;
    res = WindowsCreateString(id.c_str(), static_cast<UINT32>(id.size()), &hId);

    if (res == ERROR_SUCCESS) {
        res = this->setting->SetProperty(hId, value);
    }

    return res;
}

UINT BaseSettingItem::Invoke() {
    if (this->setting == NULL) { return ERROR_INVALID_HANDLE_STATE; };

    IPropertyValueStatics* propValueFactory;
    HSTRING rTimeClass;
    WindowsCreateString(
        RuntimeClass_Windows_Foundation_PropertyValue,
        static_cast<UINT32>(wcslen(RuntimeClass_Windows_Foundation_PropertyValue)),
        &rTimeClass
    );
    HRESULT result = GetActivationFactory(rTimeClass, &propValueFactory);

    IInspectable* rec;
    Rect baseRect { 0,0,0,0 };
    propValueFactory->CreateRect(baseRect, &rec);

    ICoreWindow* core = NULL;
    ICoreWindowStatic* spCoreWindowStatic;
    ICoreWindow* spCoreWindow;

    HSTRING strIWindowClassId;
    WindowsCreateString(
        RuntimeClass_Windows_UI_Core_CoreWindow,
        static_cast<UINT32>(wcslen(RuntimeClass_Windows_UI_Core_CoreWindow)),
        &strIWindowClassId
    );
    HRESULT hr;

    //Get the activation factory
    hr = (Windows::Foundation::GetActivationFactory(strIWindowClassId, &spCoreWindowStatic));
    if (FAILED(hr)) return true;

    //Get the current thread's object
    hr = spCoreWindowStatic->GetForCurrentThread(&spCoreWindow);
    if (FAILED(hr)) return true;

    hr = this->setting->Invoke(spCoreWindow, rec);

    return hr;
}
