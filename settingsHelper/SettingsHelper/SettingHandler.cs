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

    public class SettingHandler
    {
        private static Dictionary<string, SettingItem> settingCache = new Dictionary<string, SettingItem>();

        /// <summary>If true, don't apply any settings (but still do everything else).</summary>
        public static bool DryRun { get; set; }

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
                    settingItem = new SettingItem(payload.SettingId, SettingHandler.DryRun);
                    settingCache[payload.SettingId] = settingItem;
                }

                payload.SettingItem = settingItem;

                BindingFlags bindingFlags = BindingFlags.Instance | BindingFlags.Public | BindingFlags.IgnoreCase;
                MethodInfo getMethod = settingItem.GetType().GetMethod("GetValue", bindingFlags, null, Type.EmptyTypes, null);

                if (getMethod == null) {
                    throw new SettingFailedException("Unable to get 'GetValue' method, setting can't be verified");
                }

                var oldValue = getMethod.Invoke(settingItem, null);

                // In case of setting a new value, verify that the setting value has changed
                if (payload.Method == "GetValue")
                {
                    result.ReturnValue = oldValue;
                }
                else
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

                    try
                    {
                        object expectedValue = null;
                        if (payload.Parameters.Length == 1 && payload.Method == "SetValue")
                        {
                            Type paramType = paramTypes[0];
                            expectedValue = Convert.ChangeType(payload.Parameters[0], paramType);
                        }
                        else
                        {
                            if (payload.ExpectedValue != null) {
                                Type expValueType = payload.ExpectedValue.GetType();
                                expectedValue = Convert.ChangeType(payload.ExpectedValue, expValueType);
                            }
                            else
                            {
                                throw new SettingFailedException("Unable to verify setting due to missing 'ExpectedValue' field in payload");
                            }
                        }

                        if (!expectedValue.Equals(oldValue))
                        {
                            result.ReturnValue = method.Invoke(settingItem, payload.Parameters);

                            if (!(payload.Async ?? true))
                            {
                                settingItem.WaitForCompletion(5);
                            }

                            var newValue = getMethod.Invoke(settingItem, null);

                            if (!expectedValue.Equals(newValue))
                            {
                                throw new SettingFailedException("Unable to set the provided setting value");
                            }
                        }
                        else
                        {
                            result.ReturnValue = oldValue;
                        }
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
        [DataMember(Name = "expectedValue")]
        public object ExpectedValue { get; private set; }
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
