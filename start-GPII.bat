set DIRPMT=c:\pmt
set DIRGPII=c:\gpii

cd %DIRGPII%\node_modules\universal\testData\deviceReporter
copy GPII-installedSolutions.json installedSolutions.json

cd %DIRGPII%\windows

node gpii.js

