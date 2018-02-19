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
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.Serialization;
    using Microsoft.Win32;

    /// <summary>
    /// Defines the <see cref="Program" />
    /// </summary>
    public class Program
    {
        /// <summary>Entry point</summary>
        /// <param name="args">The <see cref="string[]"/></param>
        internal static void Main(string[] args)
        {
            string inputFile = null;

            if (args != null && args.Length > 0)
            {
                bool proceed = false;
                switch (args[0])
                {
                    case "-test":
                        SettingHandler.DryRun = true;
                        proceed = true;
                        break;

                    case "-list-all":
                    case "-list":
                        bool listAll = args[0] == "-list-all";
                        string path = SettingItem.RegistryPath.Replace("HKEY_LOCAL_MACHINE\\", string.Empty);

                        using (RegistryKey regKey = Registry.LocalMachine.OpenSubKey(path, false))
                        {
                            foreach (string key in regKey.GetSubKeyNames())
                            {
                                string settingPath = Path.Combine(SettingItem.RegistryPath, key);
                                string typeName = Registry.GetValue(settingPath, "Type", null) as string;
                                SettingType type;
                                if (Enum.TryParse(typeName, true, out type))
                                {
                                    if (listAll || (type != SettingType.Custom && type != SettingType.SettingCollection))
                                    {
                                        Console.WriteLine("{0}: {1}", key, type);
                                    }
                                }
                            }
                        }

                        break;

                    case "-methods":
                        MethodInfo[] methods = typeof(SettingItem).GetMethods(
                            BindingFlags.Instance | BindingFlags.Public | BindingFlags.FlattenHierarchy);
                        foreach (MethodInfo method in methods)
                        {
                            if (method.IsExposed())
                            {
                                IEnumerable<string> paras = method.GetParameters().Select(p =>
                                {
                                    return string.Format(CultureInfo.InvariantCulture, "{1}: {0}", p.ParameterType.Name, p.Name);
                                });
                                Console.WriteLine("{1}({2}): {0}", method.ReturnType.Name, method.Name, string.Join(", ", paras));
                            }
                        }

                        break;

                    case "-file":
                        inputFile = string.Join(" ", args.Skip(1));
                        proceed = true;
                        break;

                    default:
                        Console.Error.WriteLine("Unknown option '{0}'", args);
                        break;
                }

                if (!proceed)
                {
                    return;
                }
            }

            Stream input;
            if (inputFile == null)
            {
                input = Console.OpenStandardInput();
            }
            else
            {
                input = File.OpenRead(inputFile);
            }

            IEnumerable<Payload> payloads = null;

            using (input)
            {
                try
                {
                    payloads = Payload.FromStream(input);
                }
                catch (SerializationException e)
                {
                    Console.Error.Write("Invalid JSON: ");
                    Console.Error.WriteLine((e.InnerException ?? e).Message);
                    Environment.ExitCode = 1;
                    return;
                }
            }

            bool first = true;
            Console.Write("[");
            foreach (Payload payload in payloads)
            {
                if (!first)
                {
                    Console.WriteLine(",");
                }

                first = false;

                Result result = SettingHandler.Apply(payload);
                Console.Write(result.ToString());
            }

            Console.Write("]");

            // Wait for them all to complete.
            const long Timeout = 5000;
            Stopwatch timer = new Stopwatch();
            timer.Start();
            foreach (Payload payload in payloads.Where(p => p != null && p.SettingItem != null))
            {
                int t = (int)(Timeout - timer.ElapsedMilliseconds);
                if (t <= 0)
                {
                    break;
                }

                payload.SettingItem.WaitForCompletion(t);
            }
        }
    }
}
