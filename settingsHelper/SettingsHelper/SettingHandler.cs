/*
 * Helper for the System Settings Handler.
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
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.Serialization;
    using System.Runtime.Serialization.Json;
    using System.Threading.Tasks;

    public struct SetInfo {
        public MethodInfo method { get; set; }
        public Type[] paramTypes { get; set; }

        public SetInfo(MethodInfo m, Type[] p) : this() {
            this.method = m;
            this.paramTypes = p;
        }
    }

    public class SettingHandler
    {
        private static Dictionary<string, SettingItem> settingCache = new Dictionary<string, SettingItem>();

        /// <summary>If true, don't apply any settings (but still do everything else).</summary>
        public static bool DryRun { get; set; }

        /// <summary>Returns the desired method to call and it's parameters types, as specified in the payload parameter.</summary>
        /// <param name="payload">The payload specifying the parameters and the method that wants to be called. </param>
        /// <param name="bindingFlags">The flags that will be used for trying to get the desired method through reflection.</param>
        /// <param name="settingItem">The object from which obtain the method to be called. </param>
        /// <returns>A SettingInfo object with the method to be called and an array with it's type parameters". </returns>
        public static SetInfo GetDesiredMethod(Payload payload, BindingFlags bindingFlags, SettingItem settingItem)
        {
            // Get the parameter types to get the right overload.
            Type[] paramTypes = Type.EmptyTypes;
            if (payload.Parameters != null)
            {
                paramTypes = payload.Parameters.Select(p => p.GetType()).ToArray();
            }

            MethodInfo method =
                settingItem.GetType().GetMethod(payload.Method, bindingFlags, null, paramTypes, null);

            if (method == null)
            {
                // Method can't be found matching the parameter types, get any method with the same name
                // and let .Invoke worry about the parameters.
                try
                {
                    method = settingItem.GetType().GetMethod(payload.Method, bindingFlags);
                }
                catch (AmbiguousMatchException)
                {
                    method = null;
                }

                if (method == null)
                {
                    throw new SettingFailedException("Unknown method " + payload.Method);
                }
            }

            if (!method.IsExposed())
            {
                // Only use those with the "Exposed" attribute.
                throw new SettingFailedException("Not an exposed method " + payload.Method);
            }

            return new SetInfo(method, paramTypes);
        }

        /// <summary>
        /// Calls the method specified in the payload if possible.
        /// </summary>
        /// <param name="payload">The payload.</param>
        /// <param name="settingItem">The setting which method is going to be called.</param>
        /// <param name="oldValue">The value stored by the setting prior to this operation.</param>
        /// <param name="result">The place to store the result of this operation.</param>
        public static void CallDesiredMethod(Payload payload, SettingItem settingItem, object oldValue, ref Result result)
        {
            BindingFlags bindingFlags = BindingFlags.Instance | BindingFlags.Public | BindingFlags.IgnoreCase;

            // Get the proper method to call
            SetInfo setInfo = GetDesiredMethod(payload, bindingFlags, settingItem);

            object expectedValue = null;
            if (payload.Parameters.Length == 1 && payload.Method == "SetValue")
            {
                Type paramType = setInfo.paramTypes[0];
                expectedValue = Convert.ChangeType(payload.Parameters[0], paramType);
            }

            // Try to set the new value just in case of the expected value not matching the
            // old one, or trying to call a method different to "SetValue".
            if (!expectedValue.Equals(oldValue))
            {
                result.ReturnValue = setInfo.method.Invoke(settingItem, payload.Parameters);

                // Wait for completion in case of setting being Async.
                if (payload.Async ?? false)
                {
                    settingItem.WaitForCompletion(5);
                }

                // Check if the new setting value matches the expected one in case of being requested.
                if (payload.CheckResult ?? false) {
                    var newValue = settingItem.GetValue();

                    if (!expectedValue.Equals(newValue))
                    {
                        throw new SettingFailedException("Unable to set the provided setting value");
                    }
                }
            }
            else
            {
                result.ReturnValue = oldValue;
            }
        }

        /// <summary>
        /// Applies a payload, by invoking the method identified in <c>payload</c>.
        /// </summary>
        /// <param name="payload">The payload.</param>
        /// <returns>The result.</returns>
        public static Result Apply(Payload payload)
        {
            Result result = new Result(payload);
            try
            {
                if (string.IsNullOrEmpty(payload.SettingId))
                {
                    throw new SettingFailedException("settingID is required.");
                }

                if (string.IsNullOrEmpty(payload.Method))
                {
                    throw new SettingFailedException("method is required.");
                }

                SettingItem settingItem;

                // Cache the instance, incase it's re-used.
                if (!settingCache.TryGetValue(payload.SettingId, out settingItem))
                {
                    var async = payload.Async ?? false;
                    settingItem = new SettingItem(payload.SettingId, SettingHandler.DryRun, async);
                    settingCache[payload.SettingId] = settingItem;
                }

                payload.SettingItem = settingItem;

                var oldValue = settingItem.GetValue();

                // In case of setting a new value, verify that the setting value has changed
                if (payload.Method == "GetValue")
                {
                    result.ReturnValue = oldValue;
                }
                else
                {
                    try
                    {
                        CallDesiredMethod(payload, settingItem, oldValue, ref result);
                    }
                    catch (Exception e)
                    {
                        // Catching general exceptions is ok with .NET 4 because corrupted state exceptions aren't caught.
                        throw new SettingFailedException(null, e.InnerException ?? e);
                    }
                }
            }
            catch (SettingFailedException e)
            {
                result.IsError = true;
                result.ErrorMessage = e.Message;
            }

            return result;
        }
    }

    /// <summary>A setting payload.</summary>
    [DataContract]
    public class Payload
    {
        /// <summary>The Setting ID</summary>
        [DataMember(Name = "settingID")]
        public string SettingId { get; set; }

        /// <summary>The method of <c>SettingItem</c> to call.</summary>
        [DataMember(Name = "method")]
        public string Method { get; set; }

        /// <summary>The parameters to pass to the method.</summary>
        [DataMember(Name = "parameters")]
        public object[] Parameters { get; private set; }

        /// <summary> The expected value for the setting after performing the set operation.</summary>
        [DataMember(Name = "checkResult")]
        public bool? CheckResult { get; private set; }

        /// <summary>False to wait for this setting to complete.</summary>
        [DataMember(Name = "async")]
        public bool? Async { get; private set; }

        /// <summary>The setting item (set after it has been applied).</summary>
        internal SettingItem SettingItem { get; set; }

        /// <summary>Instantiates some payloads from the given stream of JSON.</summary>
        /// <param name="input">The input data.</param>
        /// <returns>Enumeration of payloads.</returns>
        public static IEnumerable<Payload> FromStream(Stream input)
        {
            // The built-in JSON library is crap, but it saves needing a dependency.
            DataContractJsonSerializer json = new DataContractJsonSerializer(typeof(Payload[]));
            return json.ReadObject(input) as Payload[] ?? Enumerable.Empty<Payload>();
        }
    }

    /// <summary>A payload result.</summary>
    [DataContract]
    public class Result
    {
        /// <summary>The setting ID.</summary>
        [DataMember(Name = "settingID", EmitDefaultValue = false)]
        public string SettingId { get; set; }

        /// <summary>true if there was an error.</summary>
        [DataMember(Name = "isError", EmitDefaultValue = false)]
        public bool IsError { get; set; }

        /// <summary>The error message.</summary>
        [DataMember(Name = "errorMessage", EmitDefaultValue = false)]
        public string ErrorMessage { get; set; }

        /// <summary>The return value.</summary>
        [DataMember(Name = "returnValue", EmitDefaultValue = true)]
        public object ReturnValue { get; set; }

        public Result(Payload payload)
        {
            this.SettingId = payload.SettingId;
        }

        public override string ToString()
        {
            DataContractJsonSerializer json = new DataContractJsonSerializer(typeof(Result));
            using (MemoryStream buffer = new MemoryStream())
            {
                json.WriteObject(buffer, this);
                buffer.Position = 0;
                using (StreamReader reader = new StreamReader(buffer))
                {
                    return reader.ReadToEnd();
                }
            }
        }
    }
}
