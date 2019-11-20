/**
 * Event handlers to react for settings changes.
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
#include "SettingItemEventHandler.h"

#include <string>
#include <iostream>

#pragma comment (lib, "WindowsApp.lib")

// -----------------------------------------------------------------------------
//                         ITypedEventHandler
// -----------------------------------------------------------------------------

//  ---------------------------  Public  ---------------------------------------

ITypedEventHandler<IInspectable*, HSTRING>::ITypedEventHandler(BOOL* c) : completed(c), counter(0) {}

//  ---------------------- Public - Inherited  ---------------------------------

HRESULT __stdcall ITypedEventHandler<IInspectable*, HSTRING>::QueryInterface(REFIID riid, void** ppvObject) {
    // Always set out parameter to NULL, validating it first.
    if (!ppvObject) {
        return E_INVALIDARG;
    }

    *ppvObject = NULL;
    if (riid == IID_IUnknown) {
        // Increment the reference count and return the pointer.
        *ppvObject = (LPVOID)this;
        AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}

ULONG __stdcall ITypedEventHandler<IInspectable*, HSTRING>::AddRef(void) {
    ULONG ulRefCount = InterlockedIncrement(&counter);
    return ulRefCount;
}

ULONG __stdcall ITypedEventHandler<IInspectable*, HSTRING>::Release(void) {
    // Decrement the object's internal counter.
    ULONG ulRefCount = InterlockedDecrement(&counter);

    if (0 == ulRefCount) {
        delete this;
    }

    return ulRefCount;
}

HRESULT __stdcall ITypedEventHandler<IInspectable*, HSTRING>::Invoke(IInspectable* sender, HSTRING arg) {
    HRESULT res = ERROR_SUCCESS;

    HSTRING hValue = NULL;
    PCWSTR pValue = L"Value";
    INT32 equal = 0;

    HRESULT createRes = WindowsCreateString(pValue, static_cast<UINT32>(wcslen(pValue)), &hValue);
    HRESULT cmpRes = WindowsCompareStringOrdinal(arg, hValue, &equal);

    if (createRes == ERROR_SUCCESS && cmpRes == ERROR_SUCCESS && equal == 0) {
        *(this->completed) = true;
    }

    WindowsDeleteString(hValue);

    return res;
}

// -----------------------------------------------------------------------------
//                         VectorEventHandler
// -----------------------------------------------------------------------------

//  ---------------------------  Public  ---------------------------------------

VectorEventHandler::VectorEventHandler(BOOL* c) : changed(c), counter(0) {}

//  ---------------------- Public - Inherited  ---------------------------------

HRESULT __stdcall VectorEventHandler::QueryInterface(REFIID riid, void ** ppvObject) {
    // Always set out parameter to NULL, validating it first.
    if (!ppvObject) {
        return E_INVALIDARG;
    }

    *ppvObject = NULL;
    if (riid == IID_IUnknown) {
        // Increment the reference count and return the pointer.
        *ppvObject = (LPVOID)this;
        AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}

ULONG __stdcall VectorEventHandler::AddRef(void) {
    ULONG ulRefCount = InterlockedIncrement(&counter);
    return ulRefCount;
}

ULONG __stdcall VectorEventHandler::Release(void) {
    // Decrement the object's internal counter.
    ULONG ulRefCount = InterlockedDecrement(&counter);

    if (0 == ulRefCount) {
        delete this;
    }

    return ulRefCount;}

HRESULT __stdcall VectorEventHandler::Invoke(IObservableVector<IInspectable*>* sender, IVectorChangedEventArgs * e) {
    HRESULT res = ERROR_SUCCESS;

    CollectionChange typeOfChange {};
    e->get_CollectionChange(&typeOfChange);

    if (typeOfChange == CollectionChange::CollectionChange_ItemChanged) {
        *(this->changed) = true;
    }

    return res;
}
