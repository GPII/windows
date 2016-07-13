using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace dotnet_windows_api
{
    class ProcessManager
    {
        private string processName;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="processName">The friendly name for the process. Any .exe extension will be automatically stripped. </param>
        public ProcessManager(string processName)
        {
            // Per https://msdn.microsoft.com/en-us/library/z3w4xdc9(v=vs.110).aspx, "The process name is a friendly name for the process, 
            // such as Outlook, that does not include the .exe extension or the path."
            this.processName = processName.Replace(".exe", String.Empty);
        }

        /// <summary>
        /// Kills all processes that match on the process name
        /// </summary>
        public void KillAll()
        {
            var ps = Process.GetProcessesByName(this.processName);
            foreach (var p in ps)
            {
                p.Kill();
            }
        }
    }
}

