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