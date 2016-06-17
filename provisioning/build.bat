cd c:\vagrant
call npm install

mkdir c:\node_modules
cd c:\node_modules
git clone --depth 1 https://github.com/GPII/universal.git
cd universal
call npm install

call "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools_msbuild.bat"
cd c:\vagrant\listeners
msbuild listeners.sln /p:Configuration=Release /p:FrameworkPathOverride="C:\Program Files (x86)\Reference Assemblies\Microsoft\Framework\.NETFramework\v4.5.1"
