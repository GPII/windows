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

#pragma once

#include <atlbase.h>
#include <windows.foundation.h>

#include <string>
#include <map>

using namespace ABI::Windows::Foundation;
using std::wstring;
using std::map;

using PropParser = HRESULT(const ATL::CComPtr<IPropertyValue>&, ATL::CComPtr<IPropertyValue>&);
using PropParserPtr = PropParser*;

const map<PropertyType, PropParserPtr>& propParsers();

/// <summary>
///  Create an IPropertyValue from a VARIANT.
/// </summary>
/// <param name="value">The VARIANT from which the IPropertyValue is going to be created.</param>
/// <param name="rValue">A reference to an IProperty value to be filled with the new created one.</param>
/// <returns>
///  ERROR_SUCCESS in case of success or one of the following error codes:
///     - E_OUTOFMEMORY from failed string creation or failed IPropertyValue creation.
///     - REGDB_E_CLASSNOTREG from failed activation factory.
///     - E_INVALIDARG the supplied variant value isn't supported to be converted into a IPropertyValue.
/// </returns>
HRESULT createPropertyValue(const VARIANT& value, ATL::CComPtr<IPropertyValue>& rValue);
/// <summary>
///  Create a VARIANT holding the value contained in the string 'value' parameter,
///  after converting it to the target 'PropertyType' supplied in the parameter 'type'.
/// </summary>
/// <param name="value">
///  The string value to be converted into the specified target 'PropertyType'.
/// </param>
/// <param name="type">
///  The target type for the parameter 'value' to be converted into.
/// </param>
/// <param name="rVariant">
///  A reference to the resulting VARIANT to be filled.
/// </param>
/// <returns>
///  ERROR_SUCCESS in case of success or E_INVALIDARG if the conversion can't be performed.
/// </returns>
HRESULT createValueVariant(const wstring& value, PropertyType type, VARIANT& rVariant);
/// <summary>
///  Compares if two IPropertyValue values are equal.
/// </summary>
/// <param name="fstProp">The first IPropertyValue to be compared.</param>
/// <param name="sndProp">The second IPropertyValue to be compared.</param>
/// <param name="rResult">A reference to a bool containing the result of the operation.</param>
/// <returns>
///  ERROR_SUCCESS in case of success or one of the following error codes:
///     - E_INVALIDARG if one of the IPropertyValues isn't properly initialized
///       its contents can't be retrieved.
///     - E_NOTIMPL if the comparison if requested for non-supported IPropertyValue contents.
/// </returns>
HRESULT equals(ATL::CComPtr<IPropertyValue> fstProp, ATL::CComPtr<IPropertyValue> sndProp, BOOL& result);
/// <summary>
///  Converts a IPropertyValue containing a String into a one containing a TimeSpan.
/// </summary>
/// <param name="strProp">The IPropertyValue to be converted into a TimeSpan.</param>
/// <param name="rValue">A reference to be filled with the created TimeSpan IPropertyValue.</param>
/// <returns> ERROR_SUCCESS in case of success or E_INVALIDARG in case of failure. </returns>
HRESULT convertToTimeSpan(const CComPtr<IPropertyValue>& strProp, ATL::CComPtr<IPropertyValue>& rValue);
/// <summary>
///  Converts a IPropertyValue containing a String into a one containing a DateTime.
/// </summary>
/// <param name="strProp">The IPropertyValue to be converted into a DateTime.</param>
/// <param name="rValue">A reference to be filled with the created DateTime IPropertyValue.</param>
/// <returns> ERROR_SUCCESS in case of success or E_INVALIDARG in case of failure. </returns>
HRESULT convertToDate(const CComPtr<IPropertyValue>& strProp, ATL::CComPtr<IPropertyValue>& rValue);
