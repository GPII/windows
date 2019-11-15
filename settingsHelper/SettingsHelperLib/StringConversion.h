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