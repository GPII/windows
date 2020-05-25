/**
 * Datatypes representing payload and functions.
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
#include "Payload.h"
#include "IPropertyValueUtils.h"

#include <windows.foundation.h>
#include <windows.foundation.collections.h>
#include <windows.data.json.h>
#include <atlbase.h>
#include <utility>

#include <iostream>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Data::Json;

using std::pair;

#pragma comment (lib, "WindowsApp.lib")

// -----------------------------------------------------------------------------
//                             Parameter
// -----------------------------------------------------------------------------

//  ---------------------------  Public  ---------------------------------------

Parameter::Parameter() {}

Parameter::Parameter(pair<wstring, ATL::CComPtr<IPropertyValue>> _objIdVal) :
    oIdVal(_objIdVal), isObject(true), isEmpty(false) {}

Parameter::Parameter(ATL::CComPtr<IPropertyValue> _iPropVal) :
    iPropVal(_iPropVal), isObject(false), isEmpty(false) {}

// -----------------------------------------------------------------------------
//                               Action
// -----------------------------------------------------------------------------

/// <summary>
///  Check that the members with which an action is going to be constructed are
///  valid.
/// </summary>
/// <param name="method">The method for the action.</param>
/// <param name="params">The parameters for the action</param>
/// <returns>
///  ERROR_SUCCESS in case of success or E_INVALIDARG in case of invalid
///  action members.
/// </returns>
HRESULT checkActionMembers(wstring method, vector<Parameter> params) {
    if (method != L"GetValue" && method != L"SetValue") {
        return E_INVALIDARG;
    }

    HRESULT errCode { ERROR_SUCCESS };

    if (params.size() > 1) {
        for (const auto& param : params) {
            bool validParam =
                param.isEmpty == false &&
                param.isObject == true &&
                param.oIdVal.first.empty() == false;

            if (validParam == false) {
                errCode = E_INVALIDARG;
                break;
            }
        }
    }

    return errCode;
}

/// <summary>
///  Helper function to create an action that ensure that it doens't contradict
///  the expected payload format.
/// </summary>
/// <param name="sId">The action target setting id.</param>
/// <param name="sMethod">The action target method.</param>
/// <param name="params">The action parameters</param>
/// <param name="rAction">A reference to the action to be filled.</param>
/// <returns>
///  ERROR_SUCCESS in case of success or E_INVALIDARG in case of parameters
///  not passing format checking.
/// </returns>
HRESULT createAction(wstring sId, wstring sMethod, vector<Parameter> params, Action& rAction) {
    HRESULT errCode { checkActionMembers(sMethod, params) };

    if (errCode == ERROR_SUCCESS) {
        rAction = Action { sId, sMethod, params };
    }

    return errCode;
}

// -----------------------------------------------------------------------------
//                  Parsing & Serialization Functions
// -----------------------------------------------------------------------------

//  ---------------------------  Parsing  --------------------------------------

enum class OpType {
    Get,
    Set
};

HRESULT getValueParam(ATL::CComPtr<IJsonValue> jValue, ATL::CComPtr<IPropertyValue>& jPropVal) {
    HRESULT res = ERROR_SUCCESS;

    ATL::CComPtr<IPropertyValueStatics> pValueFactory;
    HSTRING rTimeClass = NULL;
    JsonValueType jElemValueType = JsonValueType::JsonValueType_Null;

    res = WindowsCreateString(
        RuntimeClass_Windows_Foundation_PropertyValue,
        static_cast<UINT32>(wcslen(RuntimeClass_Windows_Foundation_PropertyValue)),
        &rTimeClass
    );

    if (res != ERROR_SUCCESS) goto cleanup;
    res = GetActivationFactory(rTimeClass, &pValueFactory);
    if (res != ERROR_SUCCESS) goto cleanup;

    res = jValue->get_ValueType(&jElemValueType);
    if (res != ERROR_SUCCESS) goto cleanup;

    if (jElemValueType == JsonValueType::JsonValueType_Boolean) {
        boolean elemValue;
        ATL::CComPtr<IPropertyValue> value;
        res = jValue->GetBoolean(&elemValue);

        if (res == ERROR_SUCCESS) {
            res = pValueFactory->CreateBoolean(elemValue, reinterpret_cast<IInspectable**>(&value));
            if (res == ERROR_SUCCESS) {
                jPropVal = value;
            }
        }
    } else if (jElemValueType == JsonValueType::JsonValueType_String) {
        HSTRING elemValue;
        ATL::CComPtr<IPropertyValue> value;
        res = jValue->GetString(&elemValue);

        if (res == ERROR_SUCCESS) {
            UINT32 bufSize { 0 };
            PCWSTR bufWSTR { WindowsGetStringRawBuffer(elemValue, &bufSize) };
            wstring elemValueStr { bufWSTR, bufSize };

            VARIANT vElemVal;
            vElemVal.vt = VARENUM::VT_BSTR;
            vElemVal.bstrVal = const_cast<BSTR>( elemValueStr.c_str() );

            res = createPropertyValue(vElemVal, value);
            if (res == ERROR_SUCCESS) {
                jPropVal = value;
            }
        }
    } else if (jElemValueType == JsonValueType::JsonValueType_Number) {
        DOUBLE elemValue;
        ATL::CComPtr<IPropertyValue> value;
        res = jValue->GetNumber(&elemValue);

        if (res == ERROR_SUCCESS) {
            res = pValueFactory->CreateDouble(elemValue, reinterpret_cast<IInspectable**>(&value));
            if (res == ERROR_SUCCESS) {
                jPropVal = value;
            }
        }
    } else {
        res = E_INVALIDARG;
    }

cleanup:

    return res;
}

HRESULT getObjectParam(OpType opType, ATL::CComPtr<IJsonObject> jObject, Parameter& param) {
    HRESULT res = ERROR_SUCCESS;

    // Tags
    HSTRING hElemIdTag = NULL;
    PCWSTR pElemIdTag = L"elemId";
    HSTRING hElemValTag = NULL;
    PCWSTR pElemValTag = L"elemVal";

    // Values
    UINT32 hElemIdSize = 0;
    HSTRING hElemId = NULL;
    ATL::CComPtr<IJsonValue> jElemVal;
    ATL::CComPtr<IPropertyValue> pElemVal;

    res = WindowsCreateString(pElemIdTag, static_cast<UINT32>(wcslen(pElemIdTag)), &hElemIdTag);
    if (res != ERROR_SUCCESS) goto cleanup;
    res = WindowsCreateString(pElemValTag, static_cast<UINT32>(wcslen(pElemValTag)), &hElemValTag);
    if (res != ERROR_SUCCESS) goto cleanup;

    res = jObject->GetNamedString(hElemIdTag, &hElemId);
    if (res != ERROR_SUCCESS) goto cleanup;

    if (opType == OpType::Get) {
        VARIANT emptyVar;
        emptyVar.vt = VARENUM::VT_EMPTY;

        res = createPropertyValue(emptyVar, pElemVal);
    } else {
        res = jObject->GetNamedValue(hElemValTag, &jElemVal);

        if (res == ERROR_SUCCESS) {
            res = getValueParam(jElemVal, pElemVal);
        }
    }

    if (res == ERROR_SUCCESS)  {
        param = Parameter {
            pair<wstring, ATL::CComPtr<IPropertyValue>> {
                wstring { WindowsGetStringRawBuffer(hElemId, &hElemIdSize) },
                pElemVal
            }
        };
    }

cleanup:
    if (hElemIdTag != NULL) WindowsDeleteString(hElemIdTag);
    if (hElemValTag != NULL) WindowsDeleteString(hElemValTag);
    if (hElemId != NULL) WindowsDeleteString(hElemId);

    return res;
}

HRESULT getMatchingType(OpType type, const ATL::CComPtr<IJsonArray> arrayObj, const UINT32 index, Parameter& param) {
    HRESULT res = ERROR_SUCCESS;

    ATL::CComPtr<IVector<IJsonValue*>> jVectorValue;
    ATL::CComPtr<IJsonValue> jValue;
    JsonValueType jElemValueType = JsonValueType::JsonValueType_Null;
    boolean isValueType = false;

    res = arrayObj->QueryInterface(__uuidof(__FIVector_1_Windows__CData__CJson__CIJsonValue_t), reinterpret_cast<void**>(&jVectorValue));
    if (res != ERROR_SUCCESS) goto cleanup;
    res = jVectorValue->GetAt(index, &jValue);
    if (res != ERROR_SUCCESS) goto cleanup;
    res = jValue->get_ValueType(&jElemValueType);
    if (res != ERROR_SUCCESS) goto cleanup;

    isValueType = jElemValueType == JsonValueType::JsonValueType_Boolean ||
        jElemValueType == JsonValueType::JsonValueType_Number ||
        jElemValueType == JsonValueType::JsonValueType_String;

    if (isValueType) {
        ATL::CComPtr<IPropertyValue> jPropVal;
        res = getValueParam(jValue, jPropVal);

        if (res == ERROR_SUCCESS) {
            param = Parameter{ jPropVal };
        }
    } else if (jElemValueType == JsonValueType::JsonValueType_Object) {
        ATL::CComPtr<IJsonObject> jObjVal;
        res = jValue->GetObject(&jObjVal);

        if (res == ERROR_SUCCESS) {
            res = getObjectParam(type, jObjVal, param);
        }
    } else {
        res = E_INVALIDARG;
    }

cleanup:

    return res;
}

HRESULT parseParameters(OpType type, const ATL::CComPtr<IJsonArray> arrayObj, vector<Parameter>& params) {
    if (arrayObj == NULL) { return E_INVALIDARG; };

    HRESULT res = ERROR_SUCCESS;

    HRESULT nextElemErr = ERROR_SUCCESS;
    UINT32 jElemIndex = 0;
    vector<Parameter> _params {};

    while (nextElemErr == ERROR_SUCCESS) {
        Parameter curParam {};
        nextElemErr = getMatchingType(type, arrayObj, jElemIndex, curParam);

        if (curParam.isEmpty == false && nextElemErr == ERROR_SUCCESS) {
            _params.push_back(curParam);
        } else if (nextElemErr != E_BOUNDS) {
            res = nextElemErr;
        }

        jElemIndex++;
    }

    if (res == ERROR_SUCCESS) {
        params = _params;
    }

    return res;
}

HRESULT parseAction(const ATL::CComPtr<IJsonObject> elemObj, Action& action) {
    if (elemObj == NULL) { return E_INVALIDARG; }

    HRESULT errCode = ERROR_SUCCESS;
    UINT32 bufLength = 0;

    // Required fields vars
    // ========================================================================

    HSTRING hSettingId = NULL;
    PCWSTR pSettingId = L"settingID";
    PCWSTR pSettingRawBuf = NULL;

    HSTRING hMethod = NULL;
    PCWSTR pMethod = L"method";
    PCWSTR pMethodRawBuf = NULL;

    HSTRING hSettingIdVal = NULL;
    HSTRING hMethodVal = NULL;

    wstring sSettingId {};
    wstring sMethod {};
    vector<Parameter> params {};

    // Optional fields vars
    // ========================================================================

    HRESULT getArrayErr = ERROR_SUCCESS;
    HSTRING hParams = NULL;
    PCWSTR pParams = L"parameters";
    ATL::CComPtr<IJsonArray> jParamsArray = NULL;

    // Extract required fields
    // ========================================================================

    errCode = WindowsCreateString(pSettingId, static_cast<UINT32>(wcslen(pSettingId)), &hSettingId);
    if (errCode != ERROR_SUCCESS) goto cleanup;
    errCode = WindowsCreateString(pMethod, static_cast<UINT32>(wcslen(pMethod)), &hMethod);
    if (errCode != ERROR_SUCCESS) goto cleanup;

    errCode = elemObj->GetNamedString(hSettingId, &hSettingIdVal);
    if (errCode != ERROR_SUCCESS) goto cleanup;

    errCode = elemObj->GetNamedString(hMethod, &hMethodVal);
    if (errCode != ERROR_SUCCESS) goto cleanup;

    pSettingRawBuf = WindowsGetStringRawBuffer(hSettingIdVal, &bufLength);
    pMethodRawBuf = WindowsGetStringRawBuffer(hMethodVal, &bufLength);

    if (pSettingRawBuf != NULL && pMethodRawBuf != NULL) {
        sSettingId = wstring(pSettingRawBuf);
        sMethod = wstring(pMethodRawBuf);
    } else {
        errCode = E_INVALIDARG;
        goto cleanup;
    }

    // Extract optional fields
    // ========================================================================

    errCode = WindowsCreateString(pParams, static_cast<UINT32>(wcslen(pParams)), &hParams);
    if (errCode != ERROR_SUCCESS) goto cleanup;

    getArrayErr = elemObj->GetNamedArray(hParams, &jParamsArray);

    // Check that the payload ins't of type "SetValue" if "parameters" isn't present.
    if (getArrayErr != ERROR_SUCCESS) {
        if (sMethod == L"SetValue") {
            errCode = WEB_E_JSON_VALUE_NOT_FOUND;
            goto cleanup;
        } else if (sMethod == L"GetValue") {
            action = Action { sSettingId, sMethod, params };
        } else {
            // TODO: Change with a more meaningful message
            errCode = E_INVALIDARG;
        }
    } else {
        if (sMethod == L"SetValue") {
            errCode = parseParameters(OpType::Set, jParamsArray, params);

            if (errCode == ERROR_SUCCESS) {
                Action _action {};
                errCode = createAction(sSettingId, sMethod, params, _action);

                if (errCode == ERROR_SUCCESS) {
                   action = _action;
                }
            }
        } else {
            errCode = parseParameters(OpType::Get, jParamsArray, params);

            if (errCode == ERROR_SUCCESS) {
                Action _action {};
                errCode = createAction(sSettingId, sMethod, params, _action);

                if (errCode == ERROR_SUCCESS) {
                    action = _action;
                }
            }
        }
    }

cleanup:
    if (hSettingId != NULL) { WindowsDeleteString(hSettingId); }
    if (hMethod != NULL) { WindowsDeleteString(hMethod); }
    if (hSettingIdVal != NULL) { WindowsDeleteString(hSettingIdVal); }
    if (hMethodVal != NULL ) { WindowsDeleteString(hMethodVal); }
    if (hParams != NULL) { WindowsDeleteString(hParams); }

    return errCode;
}

HRESULT parsePayload(const wstring & payload, vector<pair<Action, HRESULT>>& actions) {
    HRESULT res = ERROR_SUCCESS;
    vector<pair<Action, HRESULT>> _actions {};

    ATL::CComPtr<IJsonArrayStatics> jsonArrayFactory = NULL;
    HSTRING rJSONClass = NULL;
    res = WindowsCreateString(
        RuntimeClass_Windows_Data_Json_JsonArray,
        static_cast<UINT32>(wcslen(RuntimeClass_Windows_Data_Json_JsonArray)),
        &rJSONClass
    );

    GetActivationFactory(rJSONClass, &jsonArrayFactory);

    HSTRING hPayload = NULL;
    WindowsCreateString(payload.c_str(), static_cast<UINT32>(payload.size()), &hPayload);

    ATL::CComPtr<IJsonArray> jActionsArray = NULL;
    HRESULT nextElemErr = ERROR_SUCCESS;
    UINT32 jElemIndex = 0;

    res = jsonArrayFactory->Parse(hPayload, &jActionsArray);
    if (res != ERROR_SUCCESS) goto cleanup;

    while (nextElemErr == ERROR_SUCCESS) {
        IJsonObject* curElem = NULL;
        nextElemErr = jActionsArray->GetObjectAt(jElemIndex, &curElem);

        if (curElem != NULL && nextElemErr == ERROR_SUCCESS) {
            Action curAction {};
            ATL::CComPtr<IJsonObject> cCurElem { curElem };
            HRESULT errCode = parseAction(cCurElem, curAction);

            if (errCode == ERROR_SUCCESS) {
                _actions.push_back({ curAction, ERROR_SUCCESS });
            } else {
                _actions.push_back({ Action {}, errCode });
            }
        };

        jElemIndex++;
    }

    for (const auto& action : _actions) {
        if (action.second != ERROR_SUCCESS) {
            res = E_INVALIDARG;

            break;
        }
    }

    actions = _actions;

cleanup:
    if (rJSONClass) {
        WindowsDeleteString(rJSONClass);
    }
    if (hPayload) {
        WindowsDeleteString(hPayload);
    }

    return res;
}

//  ------------------------  Serialization  ----------------------------------

HRESULT serializeResult(const Result& result, std::wstring& str) {
    std::wstring resultStr {};
    HRESULT res = ERROR_SUCCESS;

    try {
        // JSON object start
        resultStr.append(L"{");

        // SettingId
        resultStr.append(L"\"settingID\": ");
        if (result.settingID.empty()) {
            resultStr.append(L"null");
        } else {
            resultStr.append(L"\"" + result.settingID + L"\"");
        }
        resultStr.append(L", ");

        // IsError
        resultStr.append(L"\"isError\": ");
        if (result.isError) {
            resultStr.append(L"true");
        } else {
            resultStr.append(L"false");
        }
        resultStr.append(L", ");

        // ErrorMessage
        resultStr.append(L"\"errorMessage\": ");
        if (result.errorMessage.empty()) {
            resultStr.append(L"null");
        } else {
            resultStr.append(L"\"" + result.errorMessage + L"\"");
        }
        resultStr.append(L", ");

        // ReturnValue
        resultStr.append(L"\"returnValue\": ");
        if (result.returnValue.empty()) {
            resultStr.append(L"null");
        } else {
            resultStr.append(result.returnValue);
        }

        // JSON object end
        resultStr.append(L"}");

        // Communicate back the result
        str = resultStr;
    } catch(std::bad_alloc&) {
        res = E_OUTOFMEMORY;
    }

    return res;
}