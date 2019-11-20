/**
 * Constants
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

namespace constants {
    const wstring& BaseRegPath() {
        static const wstring str { L"SOFTWARE\\Microsoft\\SystemSettings\\SettingId" };
        return str;
    }
    const wstring& BaseLibPath() {
        static const wstring str { L"C:\\Windows\\System32\\SettingsHandlers_nt.dll" };
        return str;
    }
    const vector<wstring>& KnownFaultyLibs() {
        static const vector<wstring> libPaths {
            L"C:\\Windows\\System32\\SettingsHandlers_WorkAccess.dll"
        };

        return libPaths;
    }
    const map<wstring, vector<wstring>>& CoupledLibs()
    {
        static const map<wstring, vector<wstring>> map {
            {
                L"C:\\Windows\\System32\\SettingsHandlers_Display.dll",
                {
                    L"C:\\Windows\\System32\\SettingsHandlers_PCDisplay.dll"
                },
            },
            {
                L"C:\\Windows\\System32\\SettingsHandlers_PCDisplay.dll",
                {
                    L"C:\\Windows\\System32\\SettingsHandlers_Display.dll"
                }
            }
        };

        return map;
    }
    const vector<wstring>& KnownFaultySettings() {
        static const vector<wstring> settingIds {
            L"SystemSettings_Accessibility_Narrator_OffWithTouchHintText",
            L"SystemSettings_BatterySaver_UsagePage_AppSource",
            L"SystemSettings_BatterySaver_UsagePage_AppsBreakdown",
            L"SystemSettings_Connections_Ethernet_Adapter_List_MUA",
            L"SystemSettings_Connections_MobileBroadband_ConnectionProfileSelection",
            L"SystemSettings_Connections_MobileBroadband_ESim_AddProfile",
            L"SystemSettings_Display_Status_SaveError",
            L"SystemSettings_Devices_RadialController_Add_CustomAppTool",
            L"SystemSettings_FindMyDevice_Error_NoMSA",
            L"SystemSettings_Gaming_BroadcastAudio_AutoEchoCancellation",
            L"SystemSettings_Notifications_HideNotificationContent",
            L"SystemSettings_Personalize_Color_ColorPrevalence",
            L"SystemSettings_Personalize_Font_Advanced_Metadata",
            L"SystemSettings_Personalize_Font_Uninstall",
            L"SystemSettings_Personalize_Font_VariableFont_Instances",
            // DLL index isn't updated, GetProcAddress fails returning an invalid address
            L"SystemSettings_QuickActions_Launcher",
            L"SystemSettings_Video_Preview_Calibration",
            L"SystemSettings_Video_Preview_HDR",
            L"SystemSettings_Video_Preview_SDR",
            L"SystemSettings_XLinks_CPL_Display_Link"
        };

        return settingIds;
    }
}