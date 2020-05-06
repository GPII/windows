/**
 * Tests for the parsing functions.
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
#include <Payload.h>

#include <windows.foundation.h>
#include <windows.data.json.h>

#pragma comment (lib, "WindowsApp.lib")

using namespace ABI::Windows::Foundation;

TEST(ParseJSONPayload, parseSampleJSON) {
    ABI::Windows::Data::Json::IJsonObjectStatics* jsonArrayFactory = NULL;
    HSTRING rJSONClass = NULL;
    WindowsCreateString(
        RuntimeClass_Windows_Data_Json_JsonObject,
        static_cast<UINT32>(wcslen(RuntimeClass_Windows_Data_Json_JsonObject)),
        &rJSONClass
    );

    GetActivationFactory(rJSONClass, &jsonArrayFactory);
    HSTRING hJSON;
    PCWSTR pJSON = L"{\"main\":\"Hello World\"}";
    WindowsCreateString(pJSON, static_cast<UINT32>(wcslen(pJSON)), &hJSON);

    ABI::Windows::Data::Json::IJsonObject* jObject = NULL;
    jsonArrayFactory->Parse(hJSON, &jObject);

    HSTRING hMainContent = NULL;
    HSTRING hMainName;
    PCWSTR pMainName = L"main";
    WindowsCreateString(pMainName, static_cast<UINT32>(wcslen(pMainName)), &hMainName);

    jObject->GetNamedString(hMainName, &hMainContent);

    UINT32 length = 0;
    PCWSTR pMainContent = WindowsGetStringRawBuffer(hMainContent, &length);

    std::wstring sMainContent{ pMainContent };

    EXPECT_EQ(std::wstring(L"Hello World"), sMainContent);
}

TEST(ParseJSONPayload, parseExamplePayload) {
    std::wstring payload { L"[ { \"main\" : \"Hello World\" } ]" };
    std::vector<pair<Action, HRESULT>> actions {};

    HRESULT res = parsePayload(payload, actions);
    EXPECT_EQ(E_INVALIDARG, res);
    EXPECT_EQ(WEB_E_JSON_VALUE_NOT_FOUND, actions.front().second);
}

const wstring getPayload = LR"foo(
[
  {
    "settingID": "SystemSettings_Accessibility_Magnifier_IsEnabled",
    "method" : "GetValue"
  }
]
)foo";

TEST(ParseJSONPayload, getPayloadMagnifier) {
    std::vector<pair<Action, HRESULT>> actions {};

    HRESULT res = parsePayload(getPayload, actions);
    EXPECT_EQ(ERROR_SUCCESS, res);
    ASSERT_EQ(1, actions.size());

    EXPECT_EQ(actions.front().first.settingID, std::wstring { L"SystemSettings_Accessibility_Magnifier_IsEnabled" });
    EXPECT_EQ(actions.front().first.method, std::wstring{ L"GetValue" });
}

const wstring setPayload = LR"foo(
[
  {
    "settingID": "SystemSettings_Accessibility_Magnifier_IsEnabled",
    "method": "SetValue",
    "parameters": [ true ]
  }
]
)foo";

TEST(ParseJSONPayload, setPayloadMagnifier) {
    std::vector<pair<Action, HRESULT>> actions {};

    HRESULT res = parsePayload(setPayload, actions);

    EXPECT_EQ(ERROR_SUCCESS, res);
    ASSERT_EQ(1, actions.size());

    EXPECT_EQ(actions.front().first.settingID, std::wstring { L"SystemSettings_Accessibility_Magnifier_IsEnabled" });
    EXPECT_EQ(actions.front().first.method, std::wstring{ L"SetValue" });

    ASSERT_EQ(1, actions.front().first.params.size());
    boolean paramValue = false;
    res = actions.front().first.params.front().iPropVal->GetBoolean(&paramValue);
    EXPECT_EQ(ERROR_SUCCESS, res);

    EXPECT_EQ(paramValue, boolean {true});
}
