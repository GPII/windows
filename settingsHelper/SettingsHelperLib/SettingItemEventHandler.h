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

#pragma once

#include <windows.foundation.h>
#include <windows.foundation.collections.h>
#include <windows.ui.core.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;

 /// <summary>
 ///  Completion handler used to detect when the value of a setting has been changed.
 /// </summary>
template <>
struct ITypedEventHandler<IInspectable*, HSTRING> : ITypedEventHandler_impl<IInspectable*, HSTRING> {
private:
    /// <summary>
    ///  Flag used to be setted when the operation over the setting
    ///  has been completed.
    /// </summary>
    BOOL* completed;
    /// <summary>
    ///  Counter used to keep track of the number of references created.
    /// </summary>
    LONG counter;

public:
    /// <summary>
    ///  Constructor taking a pointer to the flag used to report when the
    ///  operation is completed.
    /// </summary>
    /// <param name="c">Pointer to the flag.</param>
    ITypedEventHandler(BOOL* c);

    virtual HRESULT __stdcall QueryInterface(REFIID riid, void ** ppvObject) override;
    virtual ULONG __stdcall AddRef(void) override;
    virtual ULONG __stdcall Release(void) override;
    virtual HRESULT __stdcall Invoke(IInspectable* sender, HSTRING arg) override;
};

 /// <summary>
 ///  Handler used to detect when a ObservableVector that is used to hold the elements
 ///  of a SettingsCollection have been changed.
 /// </summary>
struct VectorEventHandler : VectorChangedEventHandler<IInspectable*> {
private:
    /// <summary>
    ///  Flag used to report when one of the elements of the vector
    ///  have been changed.
    /// </summary>
    BOOL* changed;
    /// <summary>
    ///  Counter used to keep track of the number of references created.
    /// </summary>
    LONG counter;

public:
    /// <summary>
    ///  Constructor taking a pointer to the flag used to report when
    ///  one item of the vector have been changed.
    /// </summary>
    /// <param name="c"></param>
    VectorEventHandler(BOOL* c);

    virtual HRESULT __stdcall QueryInterface(REFIID riid, void ** ppvObject) override;
    virtual ULONG __stdcall AddRef(void) override;
    virtual ULONG __stdcall Release(void) override;
    virtual HRESULT __stdcall Invoke(IObservableVector<IInspectable*>* sender, IVectorChangedEventArgs * e) override;
};