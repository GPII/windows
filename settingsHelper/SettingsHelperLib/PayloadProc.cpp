/**
 * Payload processing functions.
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
#include "PayloadProc.h"

HRESULT handleSettingAction(
    const wstring&  valueId,
    const Action&   action,
    SettingItem&    setting,
    wstring&        rVal
) {
    HRESULT errCode { ERROR_SUCCESS };

    // Inner settings values should be accessed just after loading the database
    // otherwise, last loaded setting is the one being accessed.
    ATL::CComPtr<IInspectable> iValue { NULL };
    errCode = setting.GetValue(valueId, iValue);

    if (errCode == ERROR_SUCCESS) {
        ATL::CComPtr<IPropertyValue> propValue {
            static_cast<IPropertyValue*>(iValue.Detach())
        };
        wstring resValueStr {};

        if (action.method == L"SetValue") {
            BOOL equalProps { false };
            ATL::CComPtr<IPropertyValue> paramValue { NULL };

            for (const auto& param : action.params) {
                if (param.isObject == true) {
                    if (param.oIdVal.first == setting.settingId) {
                        paramValue = param.oIdVal.second;
                    }
                } else {
                    paramValue = param.iPropVal;
                }
            }

            // Check if a conversion is needed for IPropertyValue param
            ATL::CComPtr<IPropertyValue> convParamValue { NULL };
            PropertyType propType { PropertyType::PropertyType_Empty };
            propValue->get_Type(&propType);

            auto parserKey = propParsers().find(propType);
            if (parserKey != propParsers().end()) {
                errCode = parserKey->second(paramValue, convParamValue);
            } else {
                convParamValue = paramValue;
            }

            if (errCode == ERROR_SUCCESS) {
                errCode = equals(propValue, convParamValue, equalProps);

                if (errCode == ERROR_SUCCESS) {
                    if (!equalProps) {
                        errCode = setting.SetValue(valueId, convParamValue);
                    }
                }

                if (errCode == ERROR_SUCCESS) {
                    errCode = toString(propValue, resValueStr);

                    if (errCode == ERROR_SUCCESS) {
                        rVal = resValueStr;
                    }
                }
            }
        } else {
            errCode = toString(propValue, resValueStr);

            if (errCode == ERROR_SUCCESS) {
                rVal = resValueStr;
            }
        }
    }

    return errCode;
}

wstring serializeReturnValues(vector<pair<wstring, wstring>> settingsValues) {
    wstring valueResult {};

    valueResult.append(L"[ ");

    for (const auto& setting : settingsValues) {
        // Create result value
        valueResult.append(L"{ ");
        valueResult.append(L"\"elemId\": \"");
        valueResult.append(setting.first);
        valueResult.append(L"\", ");
        valueResult.append(L"\"elemVal\": ");
        valueResult.append(setting.second);
        valueResult.append(L" }");

        if (&setting != &settingsValues.back()) {
            valueResult.append(L", ");
        }
    }

    valueResult.append(L" ]");

    return valueResult;
}

HRESULT handleCollectionAction(
    SettingAPI&     sAPI,
    const wstring&  valueId,
    const Action&   action,
    SettingItem&    setting,
    Result&         rResult
) {
    SettingType type { SettingType::Empty };
    setting.GetSettingType(&type);

    // Check that the supplied setting is of the proper type
    if (type != SettingType::SettingCollection) { return E_INVALIDARG; }

    HRESULT errCode { ERROR_SUCCESS };

    vector<wstring> tgtElemIds {};
    for (const auto& tgtElem : action.params) {
        tgtElemIds.push_back(tgtElem.oIdVal.first);
    }

    if (tgtElemIds.empty()) {
        errCode = E_INVALIDARG;
        rResult = Result { action.settingID, true, L"No target elements of the collection where supplied.", L"" };
    } else {
        vector<pair<wstring,wstring>> settingsValues {};
        vector<SettingItem> colSettings {};
        errCode = sAPI.getCollectionSettings(tgtElemIds, setting, colSettings);

        if (errCode == ERROR_SUCCESS) {
            for (auto& setting : colSettings) {
                wstring rStrVal {};
                errCode = handleSettingAction(valueId, action, setting, rStrVal);

                if (errCode == ERROR_SUCCESS) {
                    settingsValues.push_back({ setting.settingId, rStrVal });
                } else {
                    std::wostringstream errCodeStr {};
                    errCodeStr << std::hex << errCode;

                    rResult = Result {
                        action.settingID,
                        true,
                        L"Action over collection setting '" +
                        setting.settingId +
                        L"' failed with error code '0x" + errCodeStr.str(),
                        L""
                    };
                    break;
                }
            }
        } else {
            rResult = Result { action.settingID, true, L"Failed to get the settings collection elements.", L"" };
        }

        if (settingsValues.empty() == false && errCode == ERROR_SUCCESS) {
            wstring valueResult { serializeReturnValues(settingsValues) };
            rResult = Result { action.settingID, false, L"", valueResult };
        }
    }

    return errCode;
}

HRESULT getSettingPath(const Action& action, SettingPath& rPath) {
    HRESULT errCode { ERROR_SUCCESS };

    vector<wstring> settingIds {};
    errCode = splitSettingPath(action.settingID, settingIds);

    const auto& idsSize = settingIds.size();
    if (idsSize == 1) {
        rPath = { settingIds.front(), L"Value" };
    } else if (idsSize == 2) {
        rPath = { settingIds.front(), settingIds[1] };
    } else {
        errCode = E_INVALIDARG;
    }

    return errCode;
}

HRESULT handleAction(SettingAPI& sAPI, Action action, Result& rResult) {
    HRESULT errCode { ERROR_SUCCESS };
    wstring errMsg {};

    SettingPath settingPath {};
    errCode = getSettingPath(action, settingPath);

    if (errCode == ERROR_SUCCESS) {
        SettingItem baseSetting {};
        errCode = sAPI.loadBaseSetting(settingPath.first, baseSetting);

        if (errCode == ERROR_SUCCESS) {
            SettingType baseType { SettingType::Empty };
            baseSetting.GetSettingType(&baseType);

            if (errCode == ERROR_SUCCESS) {
                if (baseType == SettingType::SettingCollection) {
                    handleCollectionAction(sAPI, settingPath.second, action, baseSetting, rResult);
                } else {
                    wstring rVal {};
                    errCode = handleSettingAction(settingPath.second, action, baseSetting, rVal);

                    if (errCode == ERROR_SUCCESS) {
                        rResult = Result { action.settingID, false, L"", rVal };
                    } else {
                        errMsg = L"Failed to apply setting - ErrorCode: '0x";
                    }
                }
            } else {
                errMsg = L"Failed to get 'Setting Type' - ErrorCode: '0x";
            }
        } else {
            errMsg = L"Failed to load 'BaseSetting' - ErrorCode: '0x";
        }
    } else {
        errMsg = L"Failed to get 'SettingPath' - ErrorCode: '0x";
    }

    if (errCode != ERROR_SUCCESS) {
        std::wostringstream errCodeStr {};
        errCodeStr << std::hex << errCode;

        rResult = Result {
            action.settingID,
            true,
            errMsg + errCodeStr.str() + L"'",
            L""
        };
    }

    return errCode;
}

void readInputStream(wstring* payloadStr) {
    if (payloadStr == NULL) {
        ExitThread(E_INVALIDARG);
    }

    wstring cinData {};
    std::vector<wchar_t> buffer ( static_cast<size_t>(512) );

    auto& inputStream = std::wcin;

    while (true) {
        inputStream.read(buffer.data(), buffer.size());
        auto read = inputStream.gcount();

        if (read > 0) {
            cinData.append(buffer.begin(), buffer.begin() + read);
        }

        if (!inputStream.good()) { break; }
    }

    *payloadStr = cinData;
}

wstring buildOutputStr(const vector<Result>& results) {
    wstring output {};

    output.append(L"[");

    for (auto& result : results) {
        wstring resultStr {};
        HRESULT serRes = serializeResult(result, resultStr);
        output.append(resultStr);

        if (&result != &results.back()) {
            output.append(L",");
        }
    }

    output.append(L"]");

    return output;
}

wstring invalidPayloadMsg(HRESULT errCode) {
    std::wstring errMsg { L"Invalid payload - Parsing failed with error code: '0x" };
    std::wostringstream errCodeStr {};
    errCodeStr << std::hex << errCode;

    errMsg.append(errCodeStr.str());
    errMsg.append(L"'.");

    return errMsg;
}

HRESULT getInputPayload(pair<int, wchar_t**>* pInput, wstring& rPayloadStr) {
    HRESULT errCode { ERROR_SUCCESS };

    int argc { pInput->first };
    wchar_t** argv { pInput->second };

    if (argc == 3) {
        wstring payloadType { argv[1] };
        wstring filePath { argv[2] };

        if (payloadType == L"-file") {
            std::wifstream fileStream { filePath };

            if (fileStream) {
                rPayloadStr = wstring {
                    std::istreambuf_iterator<wchar_t>(fileStream),
                    std::istreambuf_iterator<wchar_t>()
                };
            }
        } else {
            errCode = E_INVALIDARG;
        }
    } else {
        HANDLE hInputReader { NULL };
        DWORD threadID { 0 };
        wstring inputStream {};

        hInputReader = CreateThread(
            NULL,
            0,
            (LPTHREAD_START_ROUTINE)readInputStream,
            (LPVOID)&inputStream,
            0,
            &threadID
        );

        WaitForSingleObject(hInputReader, 1000);

        if (!inputStream.empty()) {
            rPayloadStr = inputStream;
        }
    }

    return errCode;
}

HRESULT handlePayload(pair<int, wchar_t**>* pInput) {
    HRESULT res { ERROR_SUCCESS };
    vector<Result> results {};
    wstring payloadStr {};

    res = getInputPayload(pInput, payloadStr);

    if (res == ERROR_SUCCESS) {
        vector<pair<Action, HRESULT>> operations {};
        parsePayload(payloadStr, operations);

        SettingAPI& sAPI { LoadSettingAPI(res) };

        if (res == ERROR_SUCCESS) {
            for (const auto& action : operations) {
                if (action.second == ERROR_SUCCESS) {
                    Result actionResult {};
                    res = handleAction(sAPI, action.first, actionResult);

                    // Result should contain the error in case of failure
                    results.push_back(actionResult);
                } else {
                    wstring errMsg { invalidPayloadMsg(action.second) };

                    results.push_back(
                        Result {
                            L"",
                            true,
                            errMsg,
                            L""
                        }
                    );
                }
            }
        }

        res = UnloadSettingsAPI(sAPI);
    }

    auto output = buildOutputStr(results);
    std::wcout << output << std::endl;

    return res;
}
