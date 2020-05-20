/**
 * Utilities for handling IPropertyValue types.
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

#include "IPropertyValueUtils.h"
#include "StringConversion.h"

#include <roapi.h>

#pragma comment (lib, "WindowsApp.lib")

using std::wstring;
using ATL::CComPtr;

HRESULT convertToTimeSpan(const CComPtr<IPropertyValue>& strProp, ATL::CComPtr<IPropertyValue>& rValue) {
    if (strProp == NULL) { return E_INVALIDARG; }

    // Check that parameter is of proper type
    PropertyType strType { PropertyType::PropertyType_Empty };
    strProp->get_Type(&strType);
    if (strType != PropertyType::PropertyType_String) { return E_INVALIDARG; }

    HRESULT errCode { ERROR_SUCCESS };
    IPropertyValueStatics* propValueFactory { NULL };
    HSTRING rTimeClass { NULL };
    HSTRING strValue { NULL };

    // IPropertyValue to be created
    IPropertyValue* cPropValue = NULL;

    errCode = WindowsCreateString(
        RuntimeClass_Windows_Foundation_PropertyValue,
        static_cast<UINT32>(wcslen(RuntimeClass_Windows_Foundation_PropertyValue)),
        &rTimeClass
    );
    if (errCode != ERROR_SUCCESS) { goto cleanup; }
    errCode = GetActivationFactory(rTimeClass, &propValueFactory);

    // Get the source IPropertyValue inner string
    strProp->GetString(&strValue);

    try {
        // Get inner string raw buffer
        UINT32 bufSize { 0 };
        PCWSTR bufWSTR { WindowsGetStringRawBuffer(strValue, &bufSize) };

        System::String^ timeSpanStr = gcnew System::String(bufWSTR);
        System::Globalization::CultureInfo^ culture = gcnew System::Globalization::CultureInfo { L"en-US" };
        System::TimeSpan^ timeSpan = System::TimeSpan::Parse(timeSpanStr, culture);

        // Get a representation that can be stored in a IPropertyValue
        UINT64 duration = timeSpan->Ticks;
        ABI::Windows::Foundation::TimeSpan abiTimeSpan;
        abiTimeSpan.Duration = duration;

        propValueFactory->CreateTimeSpan(abiTimeSpan, reinterpret_cast<IInspectable**>(&cPropValue));
    } catch (System::Exception^) {
        errCode = E_INVALIDARG;
    }

    if (errCode == ERROR_SUCCESS) {
        rValue.Attach(cPropValue);
    }

cleanup:
    if (rTimeClass != NULL) { WindowsDeleteString(rTimeClass); }
    if (propValueFactory != NULL) { propValueFactory->Release(); }

    return errCode;
}

HRESULT convertToDate(const CComPtr<IPropertyValue>& strProp, ATL::CComPtr<IPropertyValue>& rValue) {
    if (strProp == NULL) { return E_INVALIDARG; }

    // Check that parameter is of proper type
    PropertyType strType { PropertyType::PropertyType_Empty };
    strProp->get_Type(&strType);
    if (strType != PropertyType::PropertyType_String) { return E_INVALIDARG; }

    HRESULT errCode { ERROR_SUCCESS };
    IPropertyValueStatics* propValueFactory { NULL };
    HSTRING rTimeClass { NULL };
    HSTRING strValue { NULL };

    // IPropertyValue to be created
    IPropertyValue* cPropValue = NULL;

    errCode = WindowsCreateString(
        RuntimeClass_Windows_Foundation_PropertyValue,
        static_cast<UINT32>(wcslen(RuntimeClass_Windows_Foundation_PropertyValue)),
        &rTimeClass
    );
    if (errCode != ERROR_SUCCESS) { goto cleanup; }
    errCode = GetActivationFactory(rTimeClass, &propValueFactory);
    if (errCode != ERROR_SUCCESS) { goto cleanup; }

    // Get the source IPropertyValue inner string
    strProp->GetString(&strValue);

    try {
        // Get inner string raw buffer
        UINT32 bufSize { 0 };
        PCWSTR bufWSTR { WindowsGetStringRawBuffer(strValue, &bufSize) };

        System::String^ timeDateStr = gcnew System::String(bufWSTR);
        System::Globalization::CultureInfo^ culture = gcnew System::Globalization::CultureInfo { L"en-US" };

        System::DateTime^ dateTime = System::DateTime::Parse(timeDateStr, culture, System::Globalization::DateTimeStyles::AssumeUniversal);
        ABI::Windows::Foundation::DateTime abiDateTime;
        abiDateTime.UniversalTime = dateTime->Ticks;

        propValueFactory->CreateDateTime(abiDateTime, reinterpret_cast<IInspectable**>(&cPropValue));
    } catch (System::Exception^) {
        errCode = E_INVALIDARG;
    }

    if (errCode == ERROR_SUCCESS) {
        rValue.Attach(cPropValue);
    }

cleanup:
    if (rTimeClass != NULL) { WindowsDeleteString(rTimeClass); }
    if (propValueFactory != NULL) { propValueFactory->Release(); }

    return errCode;
}

const map<PropertyType, PropParserPtr>& propParsers() {
    static const map<PropertyType, PropParserPtr> propParsers {
        { PropertyType::PropertyType_TimeSpan, &convertToTimeSpan },
        { PropertyType::PropertyType_DateTime, &convertToDate }
    };

    return propParsers;
}

HRESULT createPropertyValue(const VARIANT& value, ATL::CComPtr<IPropertyValue>& rValue) {
    HRESULT res = { ERROR_SUCCESS };
    IPropertyValueStatics* propValueFactory = NULL;
    HSTRING rTimeClass = NULL;

    // IPropertyValue to be created
    IPropertyValue* cPropValue = NULL;

    res = WindowsCreateString(
        RuntimeClass_Windows_Foundation_PropertyValue,
        static_cast<UINT32>(wcslen(RuntimeClass_Windows_Foundation_PropertyValue)),
        &rTimeClass
    );
    if (res != ERROR_SUCCESS) { goto cleanup; }
    res = GetActivationFactory(rTimeClass, &propValueFactory);
    if (res != ERROR_SUCCESS) { goto cleanup; }

    if (value.vt == VARENUM::VT_BOOL) {
        res = propValueFactory->CreateBoolean(static_cast<boolean>(value.boolVal), reinterpret_cast<IInspectable**>(&cPropValue));
    } else if (value.vt == VARENUM::VT_BSTR) {
        BSTR bStrValue = value.bstrVal;

        HSTRING newHValue = NULL;
        res = WindowsCreateString(bStrValue, static_cast<UINT32>(wcslen(bStrValue)), &newHValue);

        if (res == ERROR_SUCCESS) {
            res = propValueFactory->CreateString(newHValue, reinterpret_cast<IInspectable**>(&cPropValue));
        }
        WindowsDeleteString(newHValue);
    } else if (value.vt == VARENUM::VT_UINT) {
        res = propValueFactory->CreateUInt32(value.uintVal, reinterpret_cast<IInspectable**>(&cPropValue));
    } else if (value.vt == VARENUM::VT_R8) {
        res = propValueFactory->CreateDouble(value.dblVal, reinterpret_cast<IInspectable**>(&cPropValue));
    } else if (value.vt == VARENUM::VT_I8) {
        res = propValueFactory->CreateInt64(value.llVal, reinterpret_cast<IInspectable**>(&cPropValue));
    } else if (value.vt == VARENUM::VT_EMPTY) {
        res = propValueFactory->CreateEmpty(reinterpret_cast<IInspectable**>(&cPropValue));
    } else {
        res = E_INVALIDARG;
    }

    if (res == ERROR_SUCCESS) {
        rValue.Attach(cPropValue);
    }

cleanup:
    if (rTimeClass != NULL) { WindowsDeleteString(rTimeClass); }
    if (propValueFactory != NULL) { propValueFactory->Release(); }

    return res;
}

HRESULT createValueVariant(const wstring& value, PropertyType type, VARIANT& rVariant) {
    if (value == L"" || type == PropertyType::PropertyType_Empty) { return E_INVALIDARG; }

    HRESULT res { ERROR_SUCCESS };

    bool stringType =
        type == PropertyType::PropertyType_String	||
        type == PropertyType::PropertyType_DateTime ||
        type == PropertyType::PropertyType_TimeSpan;

    if (type == PropertyType::PropertyType_Boolean) {
        boolean varValue = false;

        // TODO: Check if this operation can except
        if (value == L"true" || value == L"false") {
            rVariant.vt = VARENUM::VT_BOOL;

            if (value == L"true") {
                rVariant.boolVal = true;
            } else {
                rVariant.boolVal = false;
            }
        } else {
            res = E_INVALIDARG;
        }
    } else if (type == PropertyType::PropertyType_Double) {
        DOUBLE propValue { 0 };
        const wchar_t* strStart = value.c_str();
        wchar_t* strEnd = NULL;

        propValue = wcstod(strStart, &strEnd);

        if (errno == ERANGE || (value != L"0" && propValue == 0)) {
            res = E_INVALIDARG;
        } else {
            rVariant.vt = VARENUM::VT_R8;
            rVariant.dblVal = propValue;
        }
    } else if (type == PropertyType::PropertyType_Int64) {
        INT64 propValue { 0 };
        const wchar_t* strStart = value.c_str();
        std::size_t strPos = NULL;

        try {
            propValue = std::stoll(strStart, &strPos);
        } catch (std::invalid_argument) {
            res = E_INVALIDARG;
        } catch (std::out_of_range) {
            res = E_INVALIDARG;
        }

        if (res == ERROR_SUCCESS) {
            rVariant.vt = VARENUM::VT_I8;
            rVariant.llVal = propValue;
        }
    } else if (stringType) {
        wchar_t* valueStr = _wcsdup(value.c_str());

        rVariant.vt = VARENUM::VT_BSTR;
        rVariant.bstrVal = valueStr;
    } else {
        res = E_INVALIDARG;
    }

    return res;
}

HRESULT equals(ATL::CComPtr<IPropertyValue> fstProp, ATL::CComPtr<IPropertyValue> sndProp, BOOL& rResult) {
    if (fstProp == NULL || sndProp == NULL) { return false; }

    PropertyType fstPropType { PropertyType::PropertyType_Empty };
    PropertyType sndPropType { PropertyType::PropertyType_Empty };

    HRESULT errCode { ERROR_SUCCESS };
    BOOL res { false };

    errCode = fstProp->get_Type(&fstPropType);

    if (errCode == ERROR_SUCCESS) {
        errCode = sndProp->get_Type(&sndPropType);

        bool nonEmptyArgs =
            fstPropType != PropertyType::PropertyType_Empty &&
            sndPropType != PropertyType::PropertyType_Empty;

        if ((nonEmptyArgs || fstPropType == sndPropType) && errCode == ERROR_SUCCESS) {
            if (fstPropType == PropertyType::PropertyType_Boolean) {
                boolean fstPropVal { false };
                boolean sndPropVal { false };

                errCode = fstProp->GetBoolean(&fstPropVal);

                if (errCode == ERROR_SUCCESS) {
                    errCode = sndProp->GetBoolean(&sndPropVal);

                    if (errCode == ERROR_SUCCESS) {
                        res = fstPropVal == sndPropVal;
                    }
                }
            } else if (fstPropType == PropertyType::PropertyType_Double) {
                DOUBLE fstPropVal {};
                DOUBLE sndPropVal {};

                errCode = fstProp->GetDouble(&fstPropVal);

                if (errCode == ERROR_SUCCESS) {
                    errCode = sndProp->GetDouble(&sndPropVal);

                    if (errCode == ERROR_SUCCESS) {
                        res = fstPropVal == sndPropVal;
                    }
                }
            } else if (fstPropType == PropertyType::PropertyType_Int64) {
                INT64 fstPropVal {};
                INT64 sndPropVal {};

                errCode = fstProp->GetInt64(&fstPropVal);

                if (errCode == ERROR_SUCCESS) {
                    errCode = sndProp->GetInt64(&sndPropVal);

                    if (errCode == ERROR_SUCCESS) {
                        res = fstPropVal == sndPropVal;
                    }
                }
            } else if (fstPropType == PropertyType::PropertyType_Int32) {
                INT32 fstPropVal {};
                INT32 sndPropVal {};

                errCode = fstProp->GetInt32(&fstPropVal);

                if (errCode == ERROR_SUCCESS) {
                    errCode = sndProp->GetInt32(&sndPropVal);

                    if (errCode == ERROR_SUCCESS) {
                        res = fstPropVal == sndPropVal;
                    }
                }
            } else if (fstPropType == PropertyType::PropertyType_UInt64) {
                UINT64 fstPropVal {};
                UINT64 sndPropVal {};

                errCode = fstProp->GetUInt64(&fstPropVal);

                if (errCode == ERROR_SUCCESS) {
                    errCode = sndProp->GetUInt64(&sndPropVal);

                    if (errCode == ERROR_SUCCESS) {
                        res = fstPropVal == sndPropVal;
                    }
                }
            } else if (fstPropType == PropertyType::PropertyType_UInt32) {
                UINT32 fstPropVal {};
                UINT32 sndPropVal {};

                errCode = fstProp->GetUInt32(&fstPropVal);

                if (errCode == ERROR_SUCCESS) {
                    errCode = sndProp->GetUInt32(&sndPropVal);

                    if (errCode == ERROR_SUCCESS) {
                        res = fstPropVal == sndPropVal;
                    }
                }
            } else if (fstPropType == PropertyType::PropertyType_DateTime) {
                DateTime fstPropVal {};
                DateTime sndPropVal {};

                errCode = fstProp->GetDateTime(&fstPropVal);

                if (errCode == ERROR_SUCCESS) {
                    try {
                        System::DateTime^ fstDateTime = gcnew System::DateTime(fstPropVal.UniversalTime);
                        System::DateTime^ sndDateTime = gcnew System::DateTime(sndPropVal.UniversalTime);

                        res = fstDateTime->Equals(sndDateTime);
                    } catch (System::Exception^) {
                        errCode = E_INVALIDARG;
                    }
                }
            } else if (fstPropType == PropertyType::PropertyType_TimeSpan) {
                TimeSpan fstPropVal {};
                TimeSpan sndPropVal {};

                errCode = fstProp->GetTimeSpan(&fstPropVal);

                if (errCode == ERROR_SUCCESS) {
                    try {
                        System::TimeSpan^ fstTimeSpan = gcnew System::TimeSpan(fstPropVal.Duration);
                        System::TimeSpan^ sndTimeSpan = gcnew System::TimeSpan(sndPropVal.Duration);

                        res = fstTimeSpan->Equals(sndTimeSpan);
                    } catch (System::Exception^) {
                        errCode = E_INVALIDARG;
                    }
                }
            } else if (fstPropType == PropertyType::PropertyType_String) {
                HSTRING fstInnerString { NULL };
                HSTRING sndInnerString { NULL };

                errCode = fstProp->GetString(&fstInnerString);
                if (errCode == ERROR_SUCCESS) {
                    UINT32 innerStringSz { 0 };
                    LPCWSTR rawStr = WindowsGetStringRawBuffer(fstInnerString, &innerStringSz);
                    wstring fstStringValue { rawStr, innerStringSz };

                    errCode = sndProp->GetString(&sndInnerString);

                    if (errCode == ERROR_SUCCESS) {
                        LPCWSTR rawStr = WindowsGetStringRawBuffer(sndInnerString, &innerStringSz);
                        wstring sndStringValue { rawStr, innerStringSz };

                        res = fstStringValue == sndStringValue;
                    }
                }
            } else {
                // TODO: Improve error message
                errCode = E_NOTIMPL;
            }
        }
    }

    if (errCode == ERROR_SUCCESS) {
        rResult = res;
    }

    return errCode;
}
