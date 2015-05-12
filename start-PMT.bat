set DIRPMT=c:\pmt
set DIRGPII=c:\gpii

cd %DIRGPII%\node_modules\universal\testData\deviceReporter
copy PMT-installedSolutions.json installedSolutions.json
cd %DIRPMT%\prefsEditors
node start.js

pause
