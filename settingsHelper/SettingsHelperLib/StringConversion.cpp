/**
 * Converts a managed string into a unmanaged one.
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

#include "StringConversion.h"

#include <msclr/marshal_cppstd.h>

HRESULT getInnerString(System::String^ sysStr, wstring& rWString) {
    HRESULT res { ERROR_SUCCESS };

    try {
        wstring tempStr =  msclr::interop::marshal_as<wstring>(sysStr);
        rWString = tempStr;
    } catch(...) {
        res = E_INVALIDARG;
    }

    return res;
}