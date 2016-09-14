cd c:\vagrant
call npm install

call "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools.bat"
cd c:\vagrant\listeners
msbuild listeners.sln /p:Configuration=Release /p:FrameworkPathOverride="C:\Program Files (x86)\Reference Assemblies\Microsoft\Framework\.NETFramework\v4.5.1"

cd c:\vagrant\gpii\node_modules\processHandling\test
cl test-window.c
del test-window.obj
