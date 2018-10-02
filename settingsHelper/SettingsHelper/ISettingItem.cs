/*
 * Interface for the SettingItem Windows Runtime object.
 *
 * Copyright 2018 Raising the Floor - International
 *
 * Licensed under the New BSD license. You may not use this file except in
 * compliance with this License.
 *
 * The research leading to these results has received funding from the European Union's
 * Seventh Framework Programme (FP7/2007-2013)
 * under grant agreement no. 289016.
 *
 * You may obtain a copy of the License at
 * https://github.com/GPII/universal/blob/master/LICENSE.txt
 */

 namespace SettingsHelper
{
    using System;
    using System.Collections.Generic;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Interface for the settings classes, instantiated by the GetSettings exports.
    /// </summary>
    /// <remarks>
    /// Most of the information was taken from the debug symbols (PDB) for the relevent DLLs. The symbols
    /// don't describe the interface, just the classes that implement it (the "vtable"). This contains the
    /// method names (and order), and vague information on the parameters (no names, and, er, de-macro'd types).
    ///
    /// Visual Studio was used to obtain the names by first creating a method with any name, then stepping into the
    /// native code from the call with the debugger where the function name will be displayed in the disassembled code.
    ///
    /// The binding of methods isn't by name, but by order, which is why the "unknown" methods must remain.
    /// Not all methods work for some type of setting.
    /// </remarks>
    [ComImport, Guid("40C037CC-D8BF-489E-8697-D66BAA3221BF"), InterfaceType(ComInterfaceType.InterfaceIsIInspectable)]
    public interface ISettingItem
    {
        int Id { get; }
        SettingType Type { get; }
        bool IsSetByGroupPolicy { get; }
        bool IsEnabled { get; }
        bool IsApplicable { get; }

        // Not always available, sometimes looks like a resource ID
        string Description
        {
            [return: MarshalAs(UnmanagedType.HString)]
            get;
        }

        // Unknown
        bool IsUpdating { get; }

        // For Type = Boolean, List, Range, String
        [return: MarshalAs(UnmanagedType.IInspectable)]
        object GetValue(
            // Normally "Value"
            [MarshalAs(UnmanagedType.HString)] string name);

        int SetValue(
            // Normally "Value"
            [MarshalAs(UnmanagedType.HString)] string name,
            [MarshalAs(UnmanagedType.IInspectable)] object value);

        // Unknown usage
        int GetProperty(string name);
        int SetProperty(string name, object value);

        // For Type = Action - performs the action.
        IntPtr Invoke(IntPtr a, Rect b);

        // SettingChanged event
        event EventHandler<string> SettingChanged;

        // Unknown - setter for IsUpdating
        bool IsUpdating2 { set; }

        // Unknown
        int GetInitializationResult();
        int DoGenericAsyncWork();
        int StartGenericAsyncWork();
        int SetSkipConcurrentOperations(bool flag);

        // These appear to be base implementations overridden by the above.
        bool GetValue2 { get; }
        IntPtr unknown_SetValue1();
        IntPtr unknown_SetValue2();
        IntPtr unknown_SetValue3();

        // Unknown usage
        IntPtr GetNamedValue(
            [MarshalAs(UnmanagedType.HString)] string name
            //[MarshalAs(UnmanagedType.IInspectable)] object unknown
            );

        IntPtr SetNullValue();

        // For Type=List:
        IntPtr GetPossibleValues(out IList<object> value);

        // There are more unknown methods.
    }

    /// <summary>The type of setting.</summary>
    public enum SettingType
    {
        // Needs investigating
        Custom = 0,

        // Read-only
        DisplayString = 1,
        LabeledString = 2,

        // Values (use GetValue/SetValue)
        Boolean = 3,
        Range = 4,
        String = 5,
        List = 6,

        // Performs an action
        Action = 7,

        // Needs investigating
        SettingCollection = 8,
    }

    /// <summary>
    /// Used by ISettingsItem.Invoke (reason unknown).
    /// </summary>
    public struct Rect
    {
        public float X;
        public float Y;
        public float Width;
        public float Height;
    }

}
