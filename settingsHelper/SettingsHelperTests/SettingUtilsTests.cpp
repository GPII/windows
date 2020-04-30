/**
 * Tests function utilities.
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
#include <IPropertyValueUtils.h>

#include <windows.foundation.h>
#include <windows.data.json.h>

#include <libloaderapi.h>
#include <MSCorEE.h>

#include <utility>
#include <vector>
#include <string>
#include <map>

using std::vector;
using std::pair;
using std::wstring;

#pragma comment (lib, "WindowsApp.lib")
#include <metahost.h>
#pragma comment(lib, "mscoree.lib")

using namespace ABI::Windows::Foundation;

TEST(CreatePropertyValue, CreateDateFromDouble) {
    HRESULT res { ERROR_SUCCESS };

    VARIANT propVarValue {};
    ATL::CComPtr<IPropertyValue> iPropValue = NULL;
    PropertyType iPropValueType = PropertyType::PropertyType_Empty;

    // Create the proper VARIANT
    propVarValue.vt = VARENUM::VT_R8;
    propVarValue.dblVal = static_cast<DOUBLE>(8.0);

    // Create the corresponding IPropertyValue from the VARIANT contents
    res = createPropertyValue(propVarValue, iPropValue);
    EXPECT_EQ(res, ERROR_SUCCESS);

    // Check the returned PropertyType
    res = iPropValue->get_Type(&iPropValueType);
    EXPECT_EQ(res, ERROR_SUCCESS);
    EXPECT_EQ(iPropValueType, PropertyType::PropertyType_Double);

    DOUBLE retDouble { 0 };
    res = iPropValue->GetDouble(&retDouble);

    EXPECT_EQ(res, ERROR_SUCCESS);
    EXPECT_EQ(retDouble, 8);
}

TEST(CreatePropertyValue, CreateDateFromINT64) {
    HRESULT res { ERROR_SUCCESS };

    VARIANT propVarValue {};
    ATL::CComPtr<IPropertyValue> iPropValue = NULL;
    PropertyType iPropValueType = PropertyType::PropertyType_Empty;

    // Create the proper VARIANT
    propVarValue.vt = VARENUM::VT_I8;
    propVarValue.llVal = static_cast<INT64>(8);

    // Create the corresponding IPropertyValue from the VARIANT contents
    res = createPropertyValue(propVarValue, iPropValue);
    EXPECT_EQ(res, ERROR_SUCCESS);

    // Check the returned PropertyType
    res = iPropValue->get_Type(&iPropValueType);
    EXPECT_EQ(res, ERROR_SUCCESS);
    EXPECT_EQ(iPropValueType, PropertyType::PropertyType_Int64);

    INT64 retDouble { 0 };
    res = iPropValue->GetInt64(&retDouble);

    EXPECT_EQ(res, ERROR_SUCCESS);
    EXPECT_EQ(retDouble, 8);
}

TEST(CreatePropertyValue, CreateFromString) {
    HRESULT res { ERROR_SUCCESS };

    std::wstring settingDLL { L"TestValue" };
    VARIANT propVarValue {};
    ATL::CComPtr<IPropertyValue> iPropValue = NULL;

    // Create the proper VARIANT
    propVarValue.vt = VARENUM::VT_BSTR;
    propVarValue.bstrVal = const_cast<BSTR>(settingDLL.c_str());

    // Create the corresponding IPropertyValue from the VARIANT contents
    res = createPropertyValue(propVarValue, iPropValue);
    EXPECT_EQ(res, ERROR_SUCCESS);

    HSTRING iPropValueStr = NULL;
    res = iPropValue->GetString(&iPropValueStr);
    EXPECT_EQ(res, ERROR_SUCCESS);

    HSTRING hStrToCompare = NULL;
    LPWSTR pStrToCompare = L"TestValue";
    res = WindowsCreateString(pStrToCompare, static_cast<UINT32>(wcslen(pStrToCompare)), &hStrToCompare);
    EXPECT_EQ(res, ERROR_SUCCESS);

    INT32 cmpRes = 0;
    res = WindowsCompareStringOrdinal(iPropValueStr, hStrToCompare, &cmpRes);
    EXPECT_EQ(res, ERROR_SUCCESS);
    EXPECT_EQ(cmpRes, 0);
}

TEST(CreatePropertyValue, CreateTimeSpanFromString) {
    HRESULT res { ERROR_SUCCESS };

    std::wstring settingDLL { L"11:00:01" };
    VARIANT propVarValue {};
    ATL::CComPtr<IPropertyValue> sPropValue { NULL };
    PropertyType sPropValueType { PropertyType::PropertyType_Empty };
    ATL::CComPtr<IPropertyValue> iPropValue { NULL };

    // Create the proper VARIANT
    propVarValue.vt = VARENUM::VT_BSTR;
    propVarValue.bstrVal = const_cast<BSTR>(settingDLL.c_str());

    // Create the corresponding IPropertyValue from the VARIANT contents
    res = createPropertyValue(propVarValue, sPropValue);
    EXPECT_EQ(res, ERROR_SUCCESS);

    // Convert the String IPropertyValue to TimeSpan
    res = convertToTimeSpan(sPropValue, iPropValue);
    EXPECT_EQ(res, ERROR_SUCCESS);

    // Check the returned PropertyType
    PropertyType iPropTimeSpan { PropertyType::PropertyType_Empty };
    res = iPropValue->get_Type(&iPropTimeSpan);
    EXPECT_EQ(res, ERROR_SUCCESS);
    EXPECT_EQ(iPropTimeSpan, PropertyType::PropertyType_TimeSpan);

    TimeSpan abiRetTypeSpan { 0 };
    res = iPropValue->GetTimeSpan(&abiRetTypeSpan);
    EXPECT_EQ(res, ERROR_SUCCESS);

    System::TimeSpan^ rtTimeSpan = gcnew System::TimeSpan(abiRetTypeSpan.Duration);
    System::Globalization::CultureInfo^ culture = gcnew System::Globalization::CultureInfo{ L"en-US" };
    System::String^ rtStrTimeSpan = rtTimeSpan->ToString("c", culture);
    System::String^ rtStrToCompare = gcnew System::String( L"11:00:01" );

    System::StringComparer^ strComparer = System::StringComparer::Create(culture, true);
    INT32 strCmpRes = strComparer->Compare(rtStrTimeSpan, rtStrToCompare);
    EXPECT_EQ(res, ERROR_SUCCESS);
    EXPECT_EQ(strCmpRes, 0);
}

TEST(CreatePropertyValue, CreateDateFromString) {
    HRESULT res { ERROR_SUCCESS };

    std::wstring settingDLL { L"5/1/2008 6:00:00 AM +00:00" };
    VARIANT propVarValue {};
    ATL::CComPtr<IPropertyValue> sPropValue { NULL };
    ATL::CComPtr<IPropertyValue> iPropValue { NULL };
    PropertyType sPropValueType { PropertyType::PropertyType_Empty };

    // Create the proper VARIANT
    propVarValue.vt = VARENUM::VT_BSTR;
    propVarValue.bstrVal = const_cast<BSTR>(settingDLL.c_str());

    // Create the corresponding IPropertyValue from the VARIANT contents
    res = createPropertyValue(propVarValue, sPropValue);
    EXPECT_EQ(res, ERROR_SUCCESS);

    // Convert the String IPropertyValue to TimeSpan
    res = convertToDate(sPropValue, iPropValue);
    EXPECT_EQ(res, ERROR_SUCCESS);

    // Check the returned PropertyType
    PropertyType iPropDateTime { PropertyType::PropertyType_Empty };
    res = iPropValue->get_Type(&iPropDateTime);
    EXPECT_EQ(res, ERROR_SUCCESS);
    EXPECT_EQ(iPropDateTime, PropertyType::PropertyType_DateTime);

    DateTime abiRetTypeSpan { 0 };
    res = iPropValue->GetDateTime(&abiRetTypeSpan);
    EXPECT_EQ(res, ERROR_SUCCESS);

    System::DateTime^ rtDateTime = gcnew System::DateTime(abiRetTypeSpan.UniversalTime, System::DateTimeKind::Utc);
    System::Globalization::CultureInfo^ culture = gcnew System::Globalization::CultureInfo { L"en-US" };
    System::String^ rtStrTimeSpan = rtDateTime->ToString("", culture);

    System::String^ rtStrToCompare = gcnew System::String(L"5/1/2008 6:00:00 AM");
    System::DateTime^ rtDateTimeToCompare = System::DateTime::Parse(rtStrToCompare, culture, System::Globalization::DateTimeStyles::AssumeUniversal);

    INT32 strCmpRes = System::DateTime::Compare(*rtDateTime, *rtDateTimeToCompare);

    EXPECT_EQ(res, ERROR_SUCCESS);
    EXPECT_EQ(strCmpRes, 0);
}
