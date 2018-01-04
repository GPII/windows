
namespace WindowsSettings
{
    using Microsoft.Win32;
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.WindowsRuntime;

    /// <summary>
    /// Handles a setting (a wrapper for ISettingItem).
    /// </summary>
    public class SettingItem
    {
        /// <summary>Location of the setting definitions in the registry.</summary>
        internal const string RegistryPath = @"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\SystemSettings\SettingId";

        /// <summary>The name of the GetSetting export.</summary>
        private const string GetSettingExport = "GetSetting";

        /// <summary>An ISettingItem class that this class is wrapping.</summary>
        protected ISettingItem settingItem;

        /// <see cref="https://msdn.microsoft.com/library/ms684175.aspx"/>
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr LoadLibrary(string lpFileName);

        /// <see cref="https://msdn.microsoft.com/library/ms683212.aspx"/>
        [DllImport("kernel32", SetLastError = true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        /// <summary>Points to a GetSetting export.</summary>
        /// <param name="settingId">Setting ID</param>
        /// <param name="settingItem">Returns the instance.</param>
        /// <param name="n">Unknown.</param>
        /// <returns>Zero on success.</returns>
        [UnmanagedFunctionPointer(CallingConvention.Winapi)]
        private delegate IntPtr GetSettingFunc(
            [MarshalAs(UnmanagedType.HString)] string settingId,
            out ISettingItem settingItem,
            IntPtr n);

        /// <summary>The type of this setting.</summary>
        public SettingType SettingType { get; private set; }

        /// <summary>
        /// Initializes a new instance of the SettingItem class.
        /// </summary>
        /// <param name="settingId">The setting ID.</param>
        public SettingItem(string settingId)
        {
            string dllPath = GetSettingDll(settingId);
            if (dllPath == null)
            {
                throw new SettingFailedException("No such setting");
            }

            this.settingItem = this.GetSettingItem(settingId, dllPath);
            this.SettingType = this.settingItem.Type;
        }

        /// <summary>Gets the setting's value.</summary>
        [Expose]
        public object GetValue()
        {
            return this.GetValue("Value");
        }

        /// <summary>Gets the setting's value.</summary>
        /// <param name="valueName">Value name (normally "Value")</param>
        [Expose]
        public object GetValue(string valueName)
        {
            return this.settingItem.GetValue(valueName);
        }

        /// <summary>Sets the setting's value.</summary>
        /// <param name="newValue">The new value.</param>
        /// <returns>The previous value.</returns>
        [Expose]
        public object SetValue(object newValue)
        {
            return this.SetValue("Value", newValue);
        }

        /// <summary>Sets the setting's value.</summary>
        /// <param name="newValue">The new value.</param>
        /// <param name="valueName">Value name (normally "Value")</param>
        /// <returns>The previous value.</returns>
        [Expose]
        public object SetValue(string valueName, object newValue)
        {
            object old = this.GetValue(valueName);
            if (newValue != old)
            {
                this.settingItem.SetValue(valueName, newValue);
            }

            return old;
        }

        [Expose]
        /// <summary>Gets a list of possible values.</summary>
        public IEnumerable<object> GetPossibleValues()
        {
            IList<object> values;
            this.settingItem.GetPossibleValues(out values);
            return values;
        }

        /// <summary>Invokes an Action setting.</summary>
        [Expose]
        public long Invoke()
        {
            return this.Invoke(IntPtr.Zero);
        }

        /// <summary>Invokes an Action setting.</summary>
        [Expose]
        public long Invoke(long n)
        {
            return this.Invoke(new IntPtr(n));
        }

        /// <summary>Invokes an Action setting.</summary>
        [Expose]
        public long Invoke(string s)
        {
            IntPtr hstring = WindowsRuntimeMarshal.StringToHString(s);
            try
            {
                return this.Invoke(hstring);
            }
            finally
            {
                WindowsRuntimeMarshal.FreeHString(hstring);
            }
        }

        /// <summary>Invokes an Action setting.</summary>
        public long Invoke(IntPtr n)
        {
            return this.settingItem.Invoke(n, new Rect()).ToInt64();
        }

        /// <summary>Gets the "IsEnabled" value.</summary>
        [Expose]
        public bool IsEnabled()
        {
            return this.settingItem.IsEnabled;
        }

        /// <summary>Gets the "IsApplicable" value.</summary>
        [Expose]
        public bool IsApplicable()
        {
            return this.settingItem.IsApplicable;
        }

        /// <summary>Waits for the setting to finish updating (IsUpdating is false)</summary>
        /// <param name="timeout">Timeout in seconds.</param>
        /// <returns>IsUpdating value - so true means it timed-out</returns>
        public bool WaitForCompletion(int timeout)
        {
            const int interval = 100;
            timeout *= (1000 / interval);
            while (this.settingItem.IsUpdating)
            {
                if (--timeout < 0)
                {
                    break;
                }

                System.Threading.Thread.Sleep(interval);
            }

            return this.settingItem.IsUpdating;
        }

        /// <summary>Gets the DLL file that contains the class for the setting.</summary>
        /// <param name="settingId">The setting ID.</param>
        /// <returns>The path of the DLL file containing the setting class, null if the setting doesn't exist.</returns>
        private string GetSettingDll(string settingId)
        {
            object value = null;
            if (!string.IsNullOrEmpty(settingId))
            {
                string path = Path.Combine(RegistryPath, settingId);
                value = Registry.GetValue(path, "DllPath", null);
            }
            return value == null ? null : value.ToString();
        }

        /// <summary>Get an instance of ISettingItem for the given setting.</summary>
        /// <param name="settingId">The setting.</param>
        /// <param name="dllPath">The dll containing the class.</param>
        /// <returns>An ISettingItem instance for the setting.</returns>
        private ISettingItem GetSettingItem(string settingId, string dllPath)
        {
            // Load the dll.
            IntPtr lib = LoadLibrary(dllPath);
            if (lib == IntPtr.Zero)
            {
                throw new SettingFailedException("Unable to load library " + dllPath);
            }

            // Get the address of the function within the dll.
            IntPtr proc = GetProcAddress(lib, GetSettingExport);
            if (proc == IntPtr.Zero)
            {
                throw new SettingFailedException(
                    string.Format("Unable get address of {0}!{1}", dllPath, GetSettingExport));
            }

            // Create a function from the address.
            GetSettingFunc getSetting = Marshal.GetDelegateForFunctionPointer<GetSettingFunc>(proc);

            // Call it.
            ISettingItem item;
            IntPtr result = getSetting(settingId, out item, IntPtr.Zero);
            if (result != IntPtr.Zero || item == null)
            {
                throw new SettingFailedException("Unable to instantiate setting class");
            }

            return item;
        }
    }

    /// <summary>Thrown when a setting class was unable to have been initialised.</summary>
    [Serializable]
    public class SettingFailedException : Exception
    {
        public SettingFailedException() { }
        public SettingFailedException(string message)
            : base(FormatMessage(message)) { }

        public SettingFailedException(string message, Exception inner)
            : base(FormatMessage(message ?? inner.Message), inner) { }

        protected SettingFailedException(
          System.Runtime.Serialization.SerializationInfo info,
          System.Runtime.Serialization.StreamingContext context)
            : base(info, context) { }

        private static string FormatMessage(string message)
        {
            int lastError = Marshal.GetLastWin32Error();
            return (lastError == 0)
                ? message
                : String.Format("{0} (win32 error {1})", message, lastError);
        }
    }

    /// <summary>
    /// A method with this attribute is exposed for use by the payload JSON.
    /// </summary>
    [System.AttributeUsage(AttributeTargets.Method, Inherited = false, AllowMultiple = true)]
    sealed class ExposeAttribute : Attribute
    {
    }

    internal static class ExposeExtension
    {
        /// <summary>
        /// Determines if the method is exposed, by having the Exposed attribute.
        /// </summary>
        /// <param name="method">The method to check.</param>
        /// <returns>true if the method has the Exposed attribute.</returns>
        public static bool IsExposed(this MethodInfo method)
        {
            return method.GetCustomAttributes<ExposeAttribute>().Any();
        }
    }
}


