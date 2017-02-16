@echo off
rem You can change the location where the settings logs are saved by replacing 
rem 'C:\gpiilogs_pilot2' in the following line by another location. Don't use a location containing spaces!!
@echo on
node ../node_modules/universal/gpii/node_modules/matchMaker/src/LogSettings.js C:\gpiilogs_pilot2
@echo off
