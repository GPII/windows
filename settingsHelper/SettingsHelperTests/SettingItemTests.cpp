/**
 * Tests setting value accessing.
 *
 * Copyright 2019 Raising the Floor - US
 *
 * Licensed under the New BSD license. You may not use this file except in
 * compliance with this License.
 *
 * You may obtain a copy of the License at
 * https://github.com/GPII/universal/blob/master/LICENSE.txt
 */

#include "pch.h"
#include <SettingItem.h>
#include <SettingUtils.h>
#include <ISettingsCollection.h>
#include <IPropertyValueUtils.h>

#include "GlobalEnvironment.h"

#include <windows.foundation.h>

#include <libloaderapi.h>
#include <MSCorEE.h>

#include <vector>
#include <string>

using std::vector;
using std::wstring;

#pragma comment (lib, "WindowsApp.lib")
#include <metahost.h>
#pragma comment(lib, "mscoree.lib")

using namespace ABI::Windows::Foundation;

HRESULT apiErrCode { ERROR_SUCCESS };
SettingAPI& sAPI { LoadSettingAPI(apiErrCode) };

void ComDLLsLibraryTearDown::SetUp() {
    ASSERT_EQ(apiErrCode, ERROR_SUCCESS);
}

void ComDLLsLibraryTearDown::TearDown() {
    UnloadSettingsAPI(sAPI);
}

// -----------------------------------------------------------------------------
//                        Getting BaseSetting Function Tests
// -----------------------------------------------------------------------------

/// <summary>
///  Test getting the corresponding setting DLL path from a setting identifier.
/// </summary>
TEST(GetSettingDLL, UsingSettingId) {
    std::wstring settingDLL { L"" };
    std::wstring settingId { L"SystemSettings_Notifications_DoNotDisturb_Toggle" };

    auto getDLLRes = getSettingDLL(settingId, settingDLL);

    EXPECT_EQ(getDLLRes, ERROR_SUCCESS);
    EXPECT_EQ(settingDLL, std::wstring { L"C:\\Windows\\System32\\SettingsHandlers_Notifications.dll" });
}

/// <summary>
///  Test loading the the corresponding setting DLL from a setting identifier.
/// </summary>
TEST(LoadBaseSetting, UsingSettingId) {
    std::wstring settingId { L"SystemSettings_Notifications_DoNotDisturb_Toggle" };
    HRESULT res { ERROR_SUCCESS };

    {
        SettingItem doNotDisturbSetting {};
        res = sAPI.loadBaseSetting(settingId, doNotDisturbSetting);
        EXPECT_EQ(res, ERROR_SUCCESS);
    }
}

// -----------------------------------------------------------------------------
//                        ParsingPaths Tests
// -----------------------------------------------------------------------------

/// <summary>
///  Test the split function used to parse the nested paths in settings identifiers.
/// </summary>
TEST(ParseSettingPath, SplitFunction) {
    wstring settingId {
        L".SystemSettings_Notifications_AppList.Microsoft\\.Windows\\.Cortana_cw5n1h2txyewy9."
    };

    vector<wstring> idsPath = split(settingId, wstring { L"." }, wstring { L'\\' });
    vector<wstring> idsResult {
        L"",
        L"SystemSettings_Notifications_AppList",
        L"Microsoft\\.Windows\\.Cortana_cw5n1h2txyewy9",
        L""
    };

    ASSERT_EQ(idsPath, idsResult);
}

/// <summary>
///  Test the path parser function with a simple setting path.
/// </summary>
TEST(ParseSettingPath, SplitSimplePath) {
    wstring settingId {
        L"SystemSettings_Notifications_AppList"
    };

    vector<wstring> idsPath = split(settingId, wstring { L"." }, wstring { L'\\' });
    vector<wstring> idsResult {
        L"SystemSettings_Notifications_AppList"
    };

    ASSERT_EQ(idsPath, idsResult);
}

/// <summary>
///  Test the path parser function with a nested setting path.
/// </summary>
TEST(ParseSettingPath, SplitSettingPath) {
    wstring settingId {
        L"SystemSettings_Notifications_AppList.Microsoft\\.Windows\\.Cortana_cw5n1h2txyewy9"
    };
    vector<wstring> idsPath {};

    HRESULT res = splitSettingPath(settingId, idsPath);
    ASSERT_EQ(res, ERROR_SUCCESS);

    vector<wstring> idsResult {
        L"SystemSettings_Notifications_AppList",
        L"Microsoft\\.Windows\\.Cortana_cw5n1h2txyewy9"
    };

    ASSERT_EQ(idsPath, idsResult);
}

// -----------------------------------------------------------------------------
//                        GetSettingItem Tests
// -----------------------------------------------------------------------------

/// <summary>
///  Test getting the value for the "Magnifier_IsEnable" setting.
/// </summary>
TEST(GetSettingItem, GetMagnifierEnable) {
    // This setting is well-known, and as many other settings, it's initialization
    // takes some time. If the thread doesn't properly wait for its value to be
    // changed, the value retrieved wont be the current one.
    wstring settingId { L"SystemSettings_Accessibility_Magnifier_IsEnabled"};
    HRESULT res { ERROR_SUCCESS };

    {
        SettingItem doNotDisturbSetting {};
        res = sAPI.loadBaseSetting(settingId, doNotDisturbSetting);
        EXPECT_EQ(res, ERROR_SUCCESS);

        if (res == ERROR_SUCCESS) {
            ATL::CComPtr<IInspectable> blueLightSetting = NULL;
            res = doNotDisturbSetting.GetValue(L"Value", blueLightSetting);

            if (res == ERROR_SUCCESS) {
                ATL::CComPtr<IPropertyValue> blueLightProp =
                    static_cast<IPropertyValue*>(blueLightSetting.Detach());
                EXPECT_FALSE(blueLightProp == NULL);

                boolean curVal { false };
                blueLightProp->GetBoolean(&curVal);
                EXPECT_EQ(curVal, boolean(false));
            }
        }
    }
}

/// <summary>
///  Test getting the value for the "Taskbar_Location" setting.
/// </summary>
TEST(GetSettingItem, GetTaskbarLocation) {
    wstring settingId { L"SystemSettings_Taskbar_Location" };

    HRESULT res { ERROR_SUCCESS };

    {
        SettingItem taskbarLocation {};
        res = sAPI.loadBaseSetting(settingId, taskbarLocation);
        EXPECT_EQ(res, ERROR_SUCCESS);

        if (res == ERROR_SUCCESS) {
            ATL::CComPtr<IInspectable> taskbarValue = NULL;
            res = taskbarLocation.GetValue(L"Value", taskbarValue);
            EXPECT_EQ(res, ERROR_SUCCESS);

            if (res == ERROR_SUCCESS) {
                ATL::CComPtr<IPropertyValue> taskbarProp =
                    static_cast<IPropertyValue*>(taskbarValue.Detach());
                EXPECT_FALSE(taskbarProp == NULL);

                UINT32 curVal { 0 };
                taskbarProp->GetUInt32(&curVal);
                // Check that the returned values is the default one
                EXPECT_EQ(curVal, UINT32 { 3 });
            }
        }
    }

}

/// <summary>
///   Try getting the 'DisplayBrightness' which is obviously not supported
///   in VMs, but it should be correctly loaded.
/// </summary>
TEST(GetSettingItem, GetInvalidSetting) {
    wstring settingId { L"SystemSettings_Accessibility_Display_DisplayBrightness"};
    HRESULT res { ERROR_SUCCESS };

    {
        SettingItem displayBrightness {};
        res = sAPI.loadBaseSetting(settingId, displayBrightness);
        EXPECT_EQ(res, ERROR_SUCCESS);
    }
}

/// <summary>
///  If base library isn't properly loaded, trying to retrieve some settings causes
///  segfaulting error.
///
///  NOTE: This setting is only available in devices with a battery, so trying to load the setting
///  fails with result E_INVALIDARG.
/// </summary>
TEST(GetSettingItem, SegfaultDueToInvalidInitialization) {
    HRESULT errCode { ERROR_SUCCESS };

    wstring settingId { L"SystemSettings_BatterySaver_UsagePage_AppsBreakdown" };
    wstring settingDLL {};
    HMODULE hLib { NULL };

    errCode = getSettingDLL(settingId, settingDLL);

    if (errCode == ERROR_SUCCESS) {
        SettingItem setting {};
        errCode = sAPI.loadBaseSetting(settingId, setting);
        EXPECT_EQ(errCode, E_INVALIDARG);
    }
}

// -----------------------------------------------------------------------------
//                        GetSettingValue Tests
// -----------------------------------------------------------------------------

///
/// <summary>
///   Test loading the color filter setting.
/// </summary>
TEST(GetSettingValue, GetColorFilterSettingValue) {
    HRESULT errCode { ERROR_SUCCESS };

    wstring settingId { L"SystemSettings_Accessibility_ColorFiltering_FilterType" };
    wstring settingDLL {};

    errCode = getSettingDLL(settingId, settingDLL);
    EXPECT_EQ(errCode, ERROR_SUCCESS);

    if (errCode == ERROR_SUCCESS) {
        SettingItem setting {};
        errCode = sAPI.loadBaseSetting(settingId, setting);
        EXPECT_EQ(errCode, ERROR_SUCCESS);
    }
}

/// <summary>
///   This setting is well-known, and as many other settings, it's initialization
///   takes some time. If the thread doesn't properly wait for its value to be
///   changed, the value retrieved wont be the current one.
///
///   This tests ensure that settings like this are being properly retrieved.
/// </summary>
TEST(GetSettingValue, GetKnownDelayedSetting) {
    wstring settingId { L"SystemSettings_Display_BlueLight_ManualToggleQuickAction"};
    HRESULT res { ERROR_SUCCESS };

    {
        SettingItem doNotDisturbSetting{};
        res = sAPI.loadBaseSetting(settingId, doNotDisturbSetting);
        EXPECT_EQ(res, ERROR_SUCCESS);

        if (res == ERROR_SUCCESS) {
            ATL::CComPtr<IInspectable> blueLightSetting = NULL;
            res = doNotDisturbSetting.GetValue(L"Value", blueLightSetting);

            if (res == ERROR_SUCCESS) {
                ATL::CComPtr<IPropertyValue> blueLightProp =
                    static_cast<IPropertyValue*>(blueLightSetting.Detach());
                EXPECT_FALSE(blueLightProp == NULL);

                boolean curVal { false };
                blueLightProp->GetBoolean(&curVal);
                EXPECT_EQ(curVal, boolean { false });
            }
        }
    }
}

/// <summary>
///   Setting property "IsApplicable" provoques a segmentation fault in case
///   of being queried during load of 'MouseCursorCustomColor', it's the
///   only known setting to exhibit this behavior. But due to this, the
///   setting is no longer queried during setting loading operation.
///   This test probes that we can safely load and access the setting value.
/// </summary>
TEST(GetSettingValue, SegfaultErrorDueToIsApplicable) {
    HRESULT errCode { ERROR_SUCCESS };

    wstring settingId { L"SystemSettings_Accessibility_MouseCursorCustomColor" };
    wstring settingDLL {};
    HMODULE hLib { NULL };

    errCode = getSettingDLL(settingId, settingDLL);

    if (errCode == ERROR_SUCCESS) {
        SettingItem setting {};
        errCode = sAPI.loadBaseSetting(settingId, setting);

        EXPECT_EQ(errCode, ERROR_SUCCESS);
        ATL::CComPtr<IInspectable> iValue { NULL };
        errCode = setting.GetValue(L"Value", iValue);

        if (errCode == ERROR_SUCCESS) {
            ATL::CComPtr<IPropertyValue> pValue {
                static_cast<IPropertyValue*>(
                    static_cast<IInspectable*>(iValue.Detach())
                )
            };
        }
    }
}

/// <summary>
///   Test trying to get setting 'XboxNetworkingAttemptFix'.
///   The DLL from this setting segfaults when unloading if
///   some of the dependent DLL are not loaded first.
///   This test proves that the DLL loading is done properly
///   even with this tricky settings.
/// </summary>
TEST(GetSettingValue, SegfaultDueToImproperDLLUnloading) {
    HRESULT errCode { ERROR_SUCCESS };

    wstring settingId { L"SystemSettings_Gaming_XboxNetworkingAttemptFix" };
    wstring settingDLL {};
    HMODULE hLib { NULL };

    errCode = getSettingDLL(settingId, settingDLL);

    if (errCode == ERROR_SUCCESS) {
        SettingItem setting {};
        errCode = sAPI.loadBaseSetting(settingId, setting);

        if (errCode == ERROR_SUCCESS) {
            ATL::CComPtr<IInspectable> iValue { NULL };
            errCode = setting.GetValue(L"Value", iValue);

            if (errCode == ERROR_SUCCESS) {
                SUCCEED();
            } else {
                // In case of setting being present, but get operation
                // fails, it should fail with 'E_NOTIMPL' error code.
                EXPECT_EQ(errCode, E_NOTIMPL);
            }
        } else {
            // In case of setting loading fail,
            // it should fail with 'E_INVALIDARG' error code.
            EXPECT_EQ(errCode, E_INVALIDARG);
        }
    } else {
        // In case of setting not being present in the system,
        // registry key should fail to be openned.
        EXPECT_EQ(errCode, ERROR_OPEN_FAILED);
    }
}

/// <summary>
///   Test getting the 'DoNotDisturb_Toggle' setting.
/// </summary>
TEST(GetSettingValue, GetDoNotDisturbToggleSetting) {
    std::wstring settingId { L"SystemSettings_Notifications_DoNotDisturb_Toggle" };
    SettingItem doNotDisturbSetting {};
    HRESULT res { ERROR_SUCCESS };

    {
        SettingItem doNotDisturbSetting {};
        res = sAPI.loadBaseSetting(settingId, doNotDisturbSetting);

        EXPECT_EQ(res, ERROR_SUCCESS);

        ATL::CComPtr<IInspectable> oldToggleValue { NULL };
        doNotDisturbSetting.GetValue(L"Value", oldToggleValue);
        EXPECT_FALSE(oldToggleValue == NULL);

        // Create new property value
        wstring tgValueStr { L"Alarms only" };
        VARIANT tgValue {};
        tgValue.vt = VARENUM::VT_BSTR;
        tgValue.bstrVal = const_cast<BSTR>(tgValueStr.c_str());

        ATL::CComPtr<IPropertyValue> newPropVal { NULL };
        res = createPropertyValue(tgValue, newPropVal);
        EXPECT_EQ(res, ERROR_SUCCESS);

        // Set the new property value
        res = doNotDisturbSetting.SetValue(L"Value", newPropVal);
        EXPECT_EQ(res, ERROR_SUCCESS);

        ATL::CComPtr<IPropertyValue> oldPropVal {
            static_cast<IPropertyValue*>(
                static_cast<IInspectable*>(oldToggleValue.Detach())
            )
        };

        // Recover the old property value
        res = doNotDisturbSetting.SetValue(L"Value", oldPropVal);
        EXPECT_EQ(res, ERROR_SUCCESS);
    }
}

/// <summary>
///  If multiple base settings are loaded without any delay between the loads,
///  a queue will be exhausted and an exception will be arised from inside the DLL.
/// </summary>
TEST(GetSettingValue, GetHDRBatteryInfoTextSetting) {
    HRESULT errCode { ERROR_SUCCESS };

    wstring settingId { L"SystemSettings_HDR_Battery_InfoText" };
    wstring settingDLL {};
    HMODULE hLib { NULL };

    errCode = getSettingDLL(settingId, settingDLL);

    if (errCode == ERROR_SUCCESS) {
        SettingItem setting {};
        errCode = sAPI.loadBaseSetting(settingId, setting);

        EXPECT_EQ(errCode, ERROR_SUCCESS);
        ATL::CComPtr<IInspectable> iValue { NULL };
        errCode = setting.GetValue(L"Value", iValue);

        if (errCode == ERROR_SUCCESS) {
            SUCCEED();
        } else {
            // In case of setting setting being present, but get operation
            // fails, it should fail with 'E_NOTIMPL' error code.
            EXPECT_EQ(errCode, E_NOTIMPL);
        }
    } else {
        // In case of setting not being present in the system,
        // registry key should fail to be openned.
        EXPECT_EQ(errCode, ERROR_OPEN_FAILED);
    }
}

/// <summary>
///   Test getting/setting the touch input activation timeout.
/// </summary>
TEST(GetSettingValue, GetInputTouchSensitivitySetting) {
    HRESULT errCode { ERROR_SUCCESS };

    wstring settingId { L"SystemSettings_Input_Touch_SetActivationTimeout" };
    wstring settingDLL {};

    errCode = getSettingDLL(settingId, settingDLL);
    EXPECT_EQ(errCode, ERROR_SUCCESS);

    if (errCode == ERROR_SUCCESS) {
        SettingItem setting {};
        errCode = sAPI.loadBaseSetting(settingId, setting);
        EXPECT_EQ(errCode, ERROR_SUCCESS);

        ATL::CComPtr<IInspectable> iValue { NULL };
        errCode = setting.GetValue(L"Value", iValue);

        if (errCode == ERROR_SUCCESS) {
            ATL::CComPtr<IPropertyValue> pValue {
                static_cast<IPropertyValue*>(
                    static_cast<IInspectable*>(iValue.Detach())
                )
            };

            VARIANT newVal {};
            newVal.vt = VARENUM::VT_BSTR;
            newVal.bstrVal = L"Medium sensitivity";

            ATL::CComPtr<IPropertyValue> newPValue { NULL };
            createPropertyValue(newVal, newPValue);

            // Set the new input sensitivity
            errCode = setting.SetValue(L"Value", newPValue);
            EXPECT_EQ(errCode, ERROR_SUCCESS);

            // Recover the old input sensitivity
            errCode = setting.SetValue(L"Value", pValue);
            EXPECT_EQ(errCode, ERROR_SUCCESS);
        }
    }
}

/// <summary>
///   Try getting the settings for a 'SettingType::SettingCollection'
///   collection type.
/// </summary>
TEST(GetSettingValue, GetCollectionValues) {
    std::wstring settingId { L"SystemSettings_Notifications_AppList" };
    HRESULT res { ERROR_SUCCESS };

    {
        SettingItem setting {};
        res = sAPI.loadBaseSetting(settingId, setting);
        EXPECT_EQ(res, ERROR_SUCCESS);

        vector<SettingItem> collectionValues {};
        res = sAPI.getCollectionSettings(setting, collectionValues);
        EXPECT_EQ(res, ERROR_SUCCESS);
    }
}

/// <summary>
///   Try getting the value for a nested setting of
///   type DBSettingItem.
/// </summary>
TEST(GetSettingValue, GetDBNestedSetting) {
    std::wstring settingId {
        L"SystemSettings_QuietMoments_On_Scheduled_Mode.SystemSettings_QuietMoments_Scheduled_Mode_StartTime"
    };
    HRESULT res { ERROR_SUCCESS };

    vector<wstring> settingIds {};
    res = splitSettingPath(settingId, settingIds);
    EXPECT_EQ(res, ERROR_SUCCESS);

    {
        SettingItem baseSetting {};
        res = sAPI.loadBaseSetting(settingIds[0], baseSetting);
        EXPECT_EQ(res, ERROR_SUCCESS);

        ATL::CComPtr<IInspectable> value;
        res = baseSetting.GetValue(settingIds[1], value);
        EXPECT_EQ(res, ERROR_SUCCESS);
        EXPECT_TRUE(value != NULL);
    }
}

/// <summary>
///   Try getting the value for a nested setting inside a collection setting.
/// </summary>
TEST(GetSettingValue, GetSettingsAppNotificationSoundToggle) {
    std::wstring settingId {
        L"SystemSettings_Notifications_AppList.Settings.SystemSettings_Notifications_AppNotificationSoundToggle"
    };
    HRESULT res { ERROR_SUCCESS };

    vector<wstring> settingIds {};
    res = splitSettingPath(settingId, settingIds);
    EXPECT_EQ(res, ERROR_SUCCESS);

    {
        SettingItem baseSetting {};
        res = sAPI.loadBaseSetting(settingIds.front(), baseSetting);
        EXPECT_EQ(res, ERROR_SUCCESS);

        SettingItem colSetting {};
        res = sAPI.getCollectionSetting(settingIds[1], baseSetting, colSetting);
        EXPECT_EQ(res, ERROR_SUCCESS);

        ATL::CComPtr<IInspectable> value;
        res = colSetting.GetValue(settingIds[2], value);
        EXPECT_EQ(res, ERROR_SUCCESS);
        EXPECT_TRUE(value != NULL);
    }
}

/// <summary>
///   Try getting and setting the value for a nested setting inside a
///   collection setting.
/// </summary>
TEST(GetSettingValue, SetSettingsAppNotificationSoundToggle) {
    std::wstring settingId {
        L"SystemSettings_Notifications_AppList.Settings.SystemSettings_Notifications_AppNotificationSoundToggle"
    };
    HRESULT res { ERROR_SUCCESS };

    vector<wstring> settingIds {};
    res = splitSettingPath(settingId, settingIds);
    EXPECT_EQ(res, ERROR_SUCCESS);

    {
        SettingItem baseSetting {};
        res = sAPI.loadBaseSetting(settingIds[0], baseSetting);
        EXPECT_EQ(res, ERROR_SUCCESS);

        {
            SettingItem colSetting {};
            res = sAPI.getCollectionSetting(settingIds[1], baseSetting, colSetting);
            EXPECT_EQ(res, ERROR_SUCCESS);

            {
                ATL::CComPtr<IInspectable> oldValue { NULL };
                res = colSetting.GetValue(settingIds[2], oldValue);
                EXPECT_EQ(res, ERROR_SUCCESS);

                VARIANT vValue {};
                vValue.vt = VARENUM::VT_BOOL;
                vValue.boolVal = true;

                ATL::CComPtr<IPropertyValue> newPropValue { NULL };
                res = createPropertyValue(vValue, newPropValue);
                EXPECT_EQ(res, ERROR_SUCCESS);

                res = colSetting.SetValue(settingIds[2], newPropValue);
                EXPECT_EQ(res, ERROR_SUCCESS);

                // Extract the IPropertyValue for the the original value
                ATL::CComPtr<IPropertyValue> oldPropValue {
                    static_cast<IPropertyValue*>(
                        static_cast<IInspectable*>(oldValue.Detach())
                    )
                };

                res = colSetting.SetValue(settingIds[2], oldPropValue);
                EXPECT_EQ(res, ERROR_SUCCESS);
            }
        }
    }
}

/// <summary>
///   Try getting and setting the value for a nested setting inside a
///   collection setting.
/// </summary>
TEST(GetSettingValue, SetSettingsAppNotifications) {
    std::wstring settingId {
        L"SystemSettings_Notifications_AppList.Settings.SystemSettings_Notifications_AppNotifications"
    };
    HRESULT res { ERROR_SUCCESS };
    vector<wstring> settingIds {};

    res = splitSettingPath(settingId, settingIds);
    EXPECT_EQ(res, ERROR_SUCCESS);

    {
        SettingItem baseSetting {};
        res = sAPI.loadBaseSetting(settingIds[0], baseSetting);
        EXPECT_EQ(res, ERROR_SUCCESS);

        {
            SettingItem colSetting {};
            res = sAPI.getCollectionSetting(settingIds[1], baseSetting, colSetting);
            EXPECT_EQ(res, ERROR_SUCCESS);

            {
                ATL::CComPtr<IInspectable> oldValue { NULL };
                res = colSetting.GetValue(settingIds[2], oldValue);
                EXPECT_EQ(res, ERROR_SUCCESS);

                VARIANT vValue {};
                vValue.vt = VARENUM::VT_BOOL;
                vValue.boolVal = true;

                ATL::CComPtr<IPropertyValue> newPropValue { NULL };
                res = createPropertyValue(vValue, newPropValue);
                EXPECT_EQ(res, ERROR_SUCCESS);

                res = colSetting.SetValue(settingIds[2], newPropValue);
                EXPECT_EQ(res, ERROR_SUCCESS);

                // Extract the IPropertyValue for the the original value
                ATL::CComPtr<IPropertyValue> oldPropValue {
                    static_cast<IPropertyValue*>(
                        static_cast<IInspectable*>(oldValue.Detach())
                    )
                };

                // Set the original value back
                res = colSetting.SetValue(settingIds[2], oldPropValue);
                EXPECT_EQ(res, ERROR_SUCCESS);
            }
        }
    }
}

/// <summary>
///   Try getting the value for a nested setting inside a collection setting.
///   As the previous test access a inner setting from a collection of a
///   different type, and check the type of the resulting value.
/// </summary>
TEST(GetSettingValue, GetNotificationMaxCollectionItems) {
    std::wstring settingId {
        L"SystemSettings_Notifications_AppList.Settings.SystemSettings_Notifications_AppNotificationMaxCollapsedGroupItemCountSetting"
    };
    HRESULT res { ERROR_SUCCESS };

    vector<wstring> settingIds {};
    res = splitSettingPath(settingId, settingIds);
    EXPECT_EQ(res, ERROR_SUCCESS);

    {
        SettingItem baseSetting {};
        res = sAPI.loadBaseSetting(settingIds.front(), baseSetting);
        EXPECT_EQ(res, ERROR_SUCCESS);

        SettingItem colSetting {};
        res = sAPI.getCollectionSetting(settingIds[1], baseSetting, colSetting);
        EXPECT_EQ(res, ERROR_SUCCESS);

        ATL::CComPtr<IInspectable> iValue;
        res = colSetting.GetValue(settingIds[2], iValue);
        ATL::CComPtr<IPropertyValue> pValue {
            static_cast<IPropertyValue*>(
                static_cast<IInspectable*>(iValue.Detach())
            )
        };

        PropertyType propType { PropertyType::PropertyType_Empty };
        pValue->get_Type(&propType);
        EXPECT_EQ(propType, PropertyType::PropertyType_String);
    }
}

#if 0
/// <summary>
///  NOTE: This test should remain commented, as it's only used for development purposes.
///
///  This test is used for trying to load all the possible settings present in the system,
///  in order to find which are faulty, or which libraries should not be used.
/// </summary>
TEST(GetAllSettingsValues, GetAllPossibleSettings) {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    HKEY hTestKey;
    HRESULT res = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("SOFTWARE\\Microsoft\\SystemSettings\\SettingId"),
        0,
        KEY_READ,
        &hTestKey
    );

    if (res == ERROR_SUCCESS) {
        vector<wstring> keys {};

        res = getRegSubKeys(hTestKey, keys);
        EXPECT_EQ(res, ERROR_SUCCESS);

        bool foundMark { false };

        for (auto& setting : keys) {
            if (setting == L"SystemSettings_Display_BlueLight_StatusInfo") {
                foundMark = true;
            }

            if (foundMark) {
                wstring settingDLL {};
                res = getSettingDLL(setting, settingDLL);

                if (res == ERROR_SUCCESS) {
                    std::wcout << setting << L"\r\n";

                    try {
                        SettingItem settingItem;
                        res = sAPI.loadBaseSetting(setting, settingItem);

                        if (res != E_NOTIMPL && res != E_INVALIDARG) {
                            EXPECT_EQ(res, ERROR_SUCCESS);
                            EXPECT_TRUE(settingItem.setting != NULL);
                        }
                    } catch(...) {}
                    // wait();
                }

                System::Threading::Thread::Sleep(300);
            }

            if (setting == L"SystemSettings_Holographic_Environment_Reset") {
                break;
            }
        }

        RegCloseKey(hTestKey);
    }

    CoUninitialize();
}
#endif