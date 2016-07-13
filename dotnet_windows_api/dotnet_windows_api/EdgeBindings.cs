using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace dotnet_windows_api
{
    /// <summary>
    /// Provides entry points to the library's functionality for EdgeJS callers
    /// </summary>
    class EdgeBindings
    {
        /// <summary>
        /// Kills all processes of a given name. ".exe" extensions will be stripped for convenience.
        /// </summary>
        /// <param name="input">A JSON object of form { "processName": "some_process.exe"}</param>
        /// <returns>nothing</returns>
        /// <remarks>
        /// Kills *all* processes matching the name. If called with "calc.exe" and multiple calc proceses are running 
        /// then they will all be killed.</remarks>
        public async Task<object> KillAllProcessesByName(dynamic input)
        {
            ProcessManager processManager = new ProcessManager((string)input.processName);
            processManager.KillAll();
            return null;
        }
    }
}

