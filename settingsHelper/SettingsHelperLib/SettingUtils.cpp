/**
 * Utility functions to handle access settings values.
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

#include "Constants.h"
#include "SettingUtils.h"
#include "StringConversion.h"
#include "DynamicSettingsDatabase.h"

#include <iterator>
#include <errno.h>
#include <string>

#include <CoreWindow.h>

// Provisional
#include <iostream>

#pragma comment (lib, "WindowsApp.lib")

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::UI::Core;

typedef int(*GetSettingFunc)(HSTRING, ISettingItem**, UINT32);

// -----------------------------------------
//      SettingAPI Helper Functions
// -----------------------------------------

BOOL isFaultySetting(const wstring& settingId) {
    const auto& pos =
        std::find(
            std::begin(constants::KnownFaultySettings()),
            std::end(constants::KnownFaultySettings()),
            settingId
        );

    return pos != std::end(constants::KnownFaultySettings());
}

BOOL isFaultyLib(const wstring& libPath) {
    const auto& pos =
        std::find(
            std::begin(constants::KnownFaultyLibs()),
            std::end(constants::KnownFaultyLibs()),
            libPath
        );

    return pos != std::end(constants::KnownFaultyLibs());
}

HRESULT getRegSubKeys(const HKEY& hKey, vector<wstring>& rKeys) {
    TCHAR    achKey[constants::MAX_KEY_LENGTH];     // Buffer for subkey name
    DWORD    cbName;                                // Size of name string
    TCHAR    achClass[MAX_PATH] = TEXT("");         // Buffer for class name
    DWORD    cchClassName = MAX_PATH;               // Size of class string
    DWORD    cSubKeys=0;                            // Number of subkeys
    DWORD    cbMaxSubKey;                           // Longest subkey size
    DWORD    cchMaxClass;                           // Longest class string
    DWORD    cValues;                               // Number of values for key
    DWORD    cchMaxValue;                           // Longest value name
    DWORD    cbMaxValueData;                        // Longest value data
    DWORD    cbSecurityDescriptor;                  // Size of security descriptor
    FILETIME ftLastWriteTime;                       // Last write time

    DWORD i, retCode;

    // Get the class name and the value count.
    retCode = RegQueryInfoKey(
        hKey,                    // key handle
        achClass,                // buffer for class name
        &cchClassName,           // size of class string
        NULL,                    // reserved
        &cSubKeys,               // number of subkeys
        &cbMaxSubKey,            // longest subkey size
        &cchMaxClass,            // longest class string
        &cValues,                // number of values for this key
        &cchMaxValue,            // longest value name
        &cbMaxValueData,         // longest value data
        &cbSecurityDescriptor,   // security descriptor
        &ftLastWriteTime);       // last write time

    // Enumerate the subkeys, until RegEnumKeyEx fails.
    if (cSubKeys) {
        for (i=0; i<cSubKeys; i++) {
            cbName = constants::MAX_KEY_LENGTH;
            retCode = RegEnumKeyEx(hKey, i,
                     achKey,
                     &cbName,
                     NULL,
                     NULL,
                     NULL,
                     &ftLastWriteTime);

            if (retCode == ERROR_SUCCESS) {
                wstring curKeyName { achKey };
                rKeys.push_back(curKeyName);
            }
        }
    }

    return retCode;
}

HRESULT setSettingValue(const VARIANT& value, SettingItem& settingItem) {
    return ERROR_SUCCESS;
}

HRESULT toString(const ATL::CComPtr<IPropertyValue>& propValue, wstring& rValueStr) {
    if (propValue == NULL) { return E_INVALIDARG; }

    HRESULT res { ERROR_SUCCESS };
    PropertyType valueType { PropertyType::PropertyType_Empty };

    res = propValue->get_Type(&valueType);
    if (res == ERROR_SUCCESS) {
        if (valueType == PropertyType::PropertyType_Boolean) {
            boolean actualVal { false };
            res = propValue->GetBoolean(&actualVal);

            if (actualVal == false) {
                rValueStr = L"false";
            } else {
                rValueStr = L"true";
            }
        } else if (valueType == PropertyType::PropertyType_Double) {
            DOUBLE actualVal { 0 };
            res = propValue->GetDouble(&actualVal);

            if (res == ERROR_SUCCESS) {
                rValueStr = std::to_wstring(actualVal);
            }
        } else if (valueType == PropertyType::PropertyType_UInt64) {
            UINT64 actualVal { 0 };
            res = propValue->GetUInt64(&actualVal);

            if (res == ERROR_SUCCESS) {
                rValueStr = std::to_wstring(actualVal);
            }
        } else if (valueType == PropertyType::PropertyType_UInt32) {
            UINT32 actualVal { 0 };
            res = propValue->GetUInt32(&actualVal);

            if (res == ERROR_SUCCESS) {
                rValueStr = std::to_wstring(actualVal);
            }
        } else if (valueType == PropertyType::PropertyType_Int64) {
            INT64 actualVal { 0 };
            res = propValue->GetInt64(&actualVal);

            if (res == ERROR_SUCCESS) {
                rValueStr = std::to_wstring(actualVal);
            }
        } else if (valueType == PropertyType::PropertyType_Int32) {
            INT32 actualVal { 0 };
            res = propValue->GetInt32(&actualVal);

            if (res == ERROR_SUCCESS) {
                rValueStr = std::to_wstring(actualVal);
            }
        } else if (valueType == PropertyType::PropertyType_DateTime) {
            DateTime actualVal {};
            res = propValue->GetDateTime(&actualVal);

            if (res == ERROR_SUCCESS) {
                try {
                    System::DateTime^ dateTime = gcnew System::DateTime(actualVal.UniversalTime);
                    System::String^ sysDateTimeStr = dateTime->ToString();

                    wstring dateTimeStr {};
                    res = getInnerString(sysDateTimeStr, dateTimeStr);

                    if (res == ERROR_SUCCESS) {
                        rValueStr = L"\"" + dateTimeStr + L"\"";
                    }
                } catch (System::Exception^) {
                    res = E_INVALIDARG;
                }
            }
        } else if (valueType == PropertyType::PropertyType_TimeSpan) {
            TimeSpan actualVal {};
            res = propValue->GetTimeSpan(&actualVal);

            if (res == ERROR_SUCCESS) {
                try {
                    System::TimeSpan^ timeSpan = gcnew System::TimeSpan(actualVal.Duration);
                    System::String^ sysTimeSpanStr = timeSpan->ToString();

                    wstring timeSpanStr {};
                    res = getInnerString(sysTimeSpanStr, timeSpanStr);

                    if (res == ERROR_SUCCESS) {
                        rValueStr = L"\"" + timeSpanStr + L"\"";
                    }
                } catch (System::Exception^) {
                    res = E_INVALIDARG;
                }
            }
        } else if (valueType == PropertyType::PropertyType_String) {
            HSTRING innerString { NULL };

            res = propValue->GetString(&innerString);
            if (res == ERROR_SUCCESS) {
                UINT32 innerStringSz { 0 };
                LPCWSTR rawStr = WindowsGetStringRawBuffer(innerString, &innerStringSz);

                rValueStr = L"\"" + wstring { rawStr, innerStringSz } + L"\"";
            }
        } else {
            // TODO: Improve error message
            res = E_INVALIDARG;
        }
    }

    return res;
}

LONG getStringRegKey(HKEY hKey, const std::wstring &strValueName, std::wstring &strValue) {
    WCHAR szBuffer[512];
    DWORD dwBufferSize = sizeof(szBuffer);
    ULONG nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);

    if (ERROR_SUCCESS == nError) {
        strValue = szBuffer;
    }

    return nError;
}

vector<wstring> split(const wstring& str, const wstring& delim, const wstring& esc_sec) {
    vector<wstring> tokens {};
    size_t s_offset = 0, v_offset = 0, pos = 0, delimSize = delim.size();
    size_t esc_pos = 0;

    while (pos != wstring::npos) {
        pos = str.find(delim, s_offset);
        if (esc_pos != wstring::npos) {
            esc_pos = str.find(esc_sec, s_offset);
        }
        s_offset = pos + delimSize;

        if (esc_pos != (pos - 1)) {
            tokens.push_back(str.substr(v_offset, pos - v_offset));
            v_offset = s_offset;
        }
    }

    return tokens;
}

HRESULT splitSettingPath(const wstring& settingPath, vector<wstring>& rIdsPath) {
    if (settingPath.empty()) { return E_INVALIDARG; }

    HRESULT res = ERROR_SUCCESS;

    vector<wstring> idsPath = split(settingPath, wstring{ L'.' }, wstring{ L'\\' });

    auto frontEmpty = idsPath.front().empty();
    auto backEmpty = idsPath.back().empty();

    if (idsPath.empty() || frontEmpty || backEmpty) {
        res = E_INVALIDARG;
    } else {
        res = ERROR_SUCCESS;
        rIdsPath = idsPath;
    }

    return res;
}

HRESULT getSettingDLL(const std::wstring& settingId, std::wstring& settingDLL) {
    HRESULT result = ERROR_SUCCESS;

    if (!settingId.empty()) {
        std::wstring settingIdPath = constants::BaseRegPath() + L"\\" + settingId;

        // Get registry key
        HKEY hKey;
        LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, settingIdPath.c_str(), 0, KEY_READ, &hKey);

        if (lRes == ERROR_SUCCESS) {
            result = getStringRegKey(hKey, L"DllPath", settingDLL);
        } else {
            result = ERROR_OPEN_FAILED;
        }
    }

    return result;
}

BOOL checkEmptyIds(const vector<wstring>& ids) {
    BOOL empty { false };

    for (const auto& id : ids) {
        if (id.empty()) {
            empty = true;
            break;
        }
    }

    return empty;
}

// -----------------------------------------------------------------------------
//                        SettingAPI Functions
// -----------------------------------------------------------------------------

//  ---------------------------  Private  --------------------------------------

SettingAPI& LoadSettingAPI(HRESULT& rErrCode) {
    static SettingAPI sAPI {};
    HRESULT errCode { ERROR_SUCCESS };

    if (sAPI.baseLibrary == NULL) {
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        HMODULE baseLibrary { LoadLibrary(L"C:\\Windows\\System32\\SettingsHandlers_nt.dll") };

        if (baseLibrary != NULL) {
            sAPI.baseLibrary = baseLibrary;
        } else {
            errCode = ERROR_ASSERTION_FAILURE;
        }
    } else {
    }

    rErrCode = errCode;

    return sAPI;
}

HRESULT UnloadSettingsAPI(SettingAPI& sAPI) {
    CoFreeUnusedLibrariesEx(0, NULL);
    CoUninitialize();

    return ERROR_SUCCESS;
}

SettingAPI::SettingAPI() {}

BOOL SettingAPI::isLibraryLoaded(wstring libraryPath) {
    BOOL res { false };

    for (const auto& library : this->loadLibraries) {
        if (library.first == libraryPath) {
            res = true;
            break;
        }
    }

    return res;
}

HRESULT SettingAPI::getLoadedLibrary(const wstring& libPath, HMODULE& rLib) {
    HRESULT errCode { ERROR_SUCCESS };
    BOOL found { false };
    HMODULE lib { NULL };

    for (const auto& libEntry : this->loadLibraries) {
        if (libEntry.first == libPath) {
            lib = libEntry.second;
            found = true;

            break;
        }
    }

    if (found) {
        rLib = lib;
    } else {
        errCode = ERROR_NOT_FOUND;
    }

    return errCode;
}

HRESULT SettingAPI::loadSettingLibrary(const wstring& settingId, HMODULE& hLib) {
    if (this->baseLibrary == NULL) { return ERROR_INVALID_HANDLE_STATE; };
    if (settingId.empty() || isFaultySetting(settingId)) { return E_INVALIDARG; };

    HRESULT res { ERROR_SUCCESS };
    wstring settingDLL { L"" };
    BOOL loaded { false };

    res = getSettingDLL(settingId, settingDLL);

    if (res == ERROR_SUCCESS) {
        if (!isFaultyLib(settingDLL)) {
            BOOL isBaseLib { settingDLL == constants::BaseLibPath() };
            loaded = isLibraryLoaded(settingDLL);

            if (loaded || isBaseLib) {
                if (isBaseLib) {
                    hLib = this->baseLibrary;
                } else {
                    HMODULE lib { NULL };
                    res = getLoadedLibrary(settingDLL, lib);

                    if (res == ERROR_SUCCESS) {
                        hLib = lib;
                    }
                }
            } else {
                res = SettingAPI::loadLibrary(settingDLL, hLib);

                if (res == ERROR_SUCCESS) {
                    for (const auto& lib : constants::CoupledLibs()) {
                        if (lib.first == settingDLL) {
                            for (const auto& coupledLib : lib.second) {
                                HMODULE _tmp {};
                                res = SettingAPI::loadLibrary(coupledLib, _tmp);

                                if (res != ERROR_SUCCESS) {
                                    // TODO: Add meaningful error message
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        } else {
            // TODO: Change for more meaningful error message
            res = E_INVALIDARG;
        }
    }

    return res;
}

HRESULT SettingAPI::loadLibrary(const wstring& libPath, HMODULE& rHLib) {
    HRESULT errCode { ERROR_SUCCESS };

    rHLib = LoadLibrary(libPath.c_str());

    if (rHLib == NULL) {
        errCode = GetLastError();
    } else {
        this->loadLibraries.push_back({ libPath, rHLib });
    }

    return errCode;
}

//  ---------------------------  Public  ---------------------------------------

HRESULT SettingAPI::loadBaseSetting(const std::wstring& settingId, SettingItem& settingItem) {
    if (this->baseLibrary == NULL) { return ERROR_INVALID_HANDLE_STATE; };
    if (settingId.empty() || isFaultySetting(settingId)) { return E_INVALIDARG; }

    HRESULT res { ERROR_SUCCESS };
    HMODULE lib { NULL };
    ISettingItem* setting { NULL };

    res = loadSettingLibrary(settingId, lib);

    if (res != ERROR_SUCCESS) {
        return res;
    }

    try {
        // TODO: Recheck the condition of failing loaded DLL.
        DWORD lastError = ERROR_SUCCESS;
        DWORD preGetLastError = GetLastError();
        GetSettingFunc getSetting = (GetSettingFunc)GetProcAddress(lib, "GetSetting");
        DWORD postGetLastError = GetLastError();

        if (preGetLastError != postGetLastError) {
            lastError = postGetLastError;
        }

        HSTRING hSettingId = NULL;
        res = WindowsCreateString(settingId.c_str(), static_cast<UINT32>(settingId.size()), &hSettingId);

        if (res == ERROR_SUCCESS && lastError == ERROR_SUCCESS && getSetting != NULL) {
            res = getSetting(hSettingId, &setting, 0);

            BOOL isUpdating { true };
            // ColorFilter setting doesn't update this field when ready.
            BOOL isEnabled { true };
            // IsApplicable' may cause segfault in certain settings.
            BOOL isApplicable { true };

            UINT counter = 0;
            while (isUpdating) {//  || (isEnabled == false )) { // && isApplicable == false)) {
                // Timer
                // ==========
                if (counter > 10) {
                    break;
                } else {
                    System::Threading::Thread::Sleep(10);
                    counter += 1;
                }

                setting->get_IsUpdating(&isUpdating);
                // setting->get_IsApplicable(&isApplicable);
                // setting->get_IsEnabled(&isEnabled);
            }

            if (res == ERROR_SUCCESS && isApplicable == TRUE && isEnabled == TRUE && isUpdating == FALSE) {
                ATL::CComPtr<ISettingItem> comSetting { NULL };
                comSetting.Attach(setting);
                settingItem = SettingItem { settingId, comSetting };
            } else {
                setting->Release();
                res = E_INVALIDARG;
            }
        } else {
            res = lastError;
        }

        WindowsDeleteString(hSettingId);

    } catch(...) {
        if (setting != NULL) {
            setting->Release();
        }
        res = E_NOTIMPL;
    }

    return res;
}

HRESULT SettingAPI::getCollectionSettings(const vector<wstring>& ids, SettingItem& collSetting, vector<SettingItem>& rSettings) {
    if (this->baseLibrary == NULL) { return ERROR_INVALID_HANDLE_STATE; };
    if (ids.empty() || checkEmptyIds(ids)) { return E_INVALIDARG; }

    SettingType type { SettingType::Empty };
    collSetting.GetSettingType(&type);
    if (type != SettingType::SettingCollection) {
        // TODO: Change with more meaningful error
        return E_INVALIDARG;
    }

    HRESULT errCode { ERROR_SUCCESS };
    UINT32 vectorSize { 0 };
    vector<wstring> _ids { ids };

    ATL::CComPtr<IInspectable> collection = NULL;
    errCode = collSetting.GetValue(L"Value", collection);

    if (errCode == ERROR_SUCCESS) {
        ATL::CComPtr<IVector<IInspectable*>> pSettingVector { NULL };
        pSettingVector.Attach(
            static_cast<IVector<IInspectable*>*>(
                static_cast<IInspectable*>(
                    collection.Detach()
                )
            )
        );

        UINT32 vectorSize { 0 };
        vector<SettingItem> settingItems {};

        errCode = pSettingVector->get_Size(&vectorSize);

        if (errCode == ERROR_SUCCESS) {
            for (UINT32 i = 0; i < vectorSize; i++) {
                ATL::CComPtr<ISettingItem> pCurSetting = NULL;
                errCode = pSettingVector->GetAt(i, reinterpret_cast<IInspectable**>(&pCurSetting));

                if (errCode == ERROR_SUCCESS) {
                    for (auto& id = _ids.begin(); id != _ids.end();) {
                        SettingItem setting { collSetting.settingId, *id, pCurSetting };
                        std::wstring curSettingDesc {};
                        std::wstring curSettingId {};

                        // TODO: We may want to check the return of this operations.
                        setting.GetDescription(curSettingDesc);
                        setting.GetId(curSettingId);

                        if (*id == curSettingId || *id == curSettingDesc) {
                            settingItems.push_back(setting);

                            // Remove the already found id
                            id = _ids.erase(id);
                        } else {
                            ++id;
                        }
                    }
                } else {
                    break;
                }
            }
        }

        if (settingItems.empty()) {
            errCode = E_INVALIDARG;
        } else {
            rSettings = settingItems;
        }
    }

    return errCode;
}

HRESULT SettingAPI::getCollectionSettings(SettingItem& collSetting, vector<SettingItem>& rSettings) {
    if (this->baseLibrary == NULL) { return ERROR_INVALID_HANDLE_STATE; };

    HRESULT errCode { ERROR_SUCCESS };
    UINT32 vectorSize { 0 };

    SettingType type { SettingType::Empty };
    errCode = collSetting.GetSettingType(&type);
    if (type != SettingType::SettingCollection) {
        // TODO: Change with more meaningful error
        return E_INVALIDARG;
    }

    ATL::CComPtr<IInspectable> collection = NULL;
    errCode = collSetting.GetValue(L"Value", collection);

    if (errCode == ERROR_SUCCESS) {
        ATL::CComPtr<IVector<IInspectable*>> pSettingVector { NULL };
        pSettingVector.Attach(
            static_cast<IVector<IInspectable*>*>(
                static_cast<IInspectable*>(
                    collection.Detach()
                )
            )
        );

        UINT32 vectorSize { 0 };
        vector<SettingItem> settingItems {};

        errCode = pSettingVector->get_Size(&vectorSize);

        if (errCode == ERROR_SUCCESS) {
            for (UINT32 i = 0; i < vectorSize; i++) {
                ATL::CComPtr<ISettingItem> pCurSetting = NULL;
                errCode = pSettingVector->GetAt(i, reinterpret_cast<IInspectable**>(&pCurSetting));

                if (errCode == ERROR_SUCCESS) {
                    HSTRING hCurSettingId { NULL };
                    errCode = pCurSetting->get_Id(&hCurSettingId);

                    if (errCode) {
                        UINT32 length { 0 };
                        LPCWSTR pStrBuffer { WindowsGetStringRawBuffer(hCurSettingId, &length) };
                        std::wstring id { pStrBuffer };
                        SettingItem setting { collSetting.settingId, id, pCurSetting };

                        settingItems.push_back(setting);
                    }
                } else {
                    break;
                }
            }
        }

        if (settingItems.empty()) {
            errCode = E_INVALIDARG;
        } else {
            rSettings = settingItems;
        }
    }

    return errCode;
}

HRESULT SettingAPI::getCollectionSetting(const wstring& id, SettingItem& settingCollection, SettingItem& rSetting) {
    if (this->baseLibrary == NULL) { return ERROR_INVALID_HANDLE_STATE; };
    if (id.empty()) { return E_INVALIDARG; }

    SettingType type { SettingType::Empty };
    settingCollection.GetSettingType(&type);
    if (type != SettingType::SettingCollection) {
        // TODO: Change with more meaningful error
        return E_INVALIDARG;
    }

    HRESULT errCode { ERROR_SUCCESS };
    vector<SettingItem> settings {};

    errCode = getCollectionSettings({ id }, settingCollection, settings);

    if (errCode == ERROR_SUCCESS && !settings.empty()) {
        rSetting = settings.front();
    } else {
        errCode = E_INVALIDARG;
    }

    return errCode;
}
