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

#pragma once

#include <string>

#include <winerror.h>

using std::wstring;

/// <summary>
///  Converts a managed System::String into an unmanaged std::wstring.
/// </summary>
/// <param name="sysStr">
///  The System::String to be converted into an unmanaged string.
/// </param>
/// <param name="rWString">
///  A reference to an wstring to be filled with the contents of the managed string.
/// </param>
/// <returns>
///  ERROR_SUCCESS in case of success or E_INVALIDARG if something failed.
/// </returns>
HRESULT getInnerString(System::String^ sysStr, wstring& rWString);