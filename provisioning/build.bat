cd c:\vagrant
call npm install

call "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools_msbuild.bat"
cd c:\vagrant\listeners
msbuild listeners.sln /p:Configuration=Release /p:FrameworkPathOverride="C:\Program Files (x86)\Reference Assemblies\Microsoft\Framework\.NETFramework\v4.5.1"

cd c:\vagrant\dotnet_windows_api
msbuild dotnet_windows_api.sln /p:Configuration=Debug
msbuild dotnet_windows_api.sln /p:Configuration=Release
