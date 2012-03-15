/*!
Windows Registry Settings Handler

Node wrapper for the RegistrySettingsHandler.cpp.

Copyright 2012 Raising the Floor - International

Licensed under the New BSD license. You may not use this file except in
compliance with this License.

You may obtain a copy of the License at
https://github.com/gpii/universal/LICENSE.txt
*/

var exec = require('child_process').exec;

var gpii = gpii || {};
gpii.settingsHandlers = gpii.settingsHandlers || {};
gpii.settingsHandlers.windowsRegistrySettingsHandler = gpii.settingsHandlers.windowsRegistrySettingsHandler || {};

(function () {
    //public functions
    gpii.settingsHandlers.windowsRegistrySettingsHandler.get = function(data) {
		//implement when the executable has this implemented
    };

    gpii.settingsHandlers.windowsRegistrySettingsHandler.set = function(data) {
    	data = JSON.stringify(data);
    	data = data.replace(/"/g, "\\\""); //escape quotes for output
		var cmd = "RegistrySettingsHandler.exe \""+data+"\"";
		console.log(cmd);
 		var proc = exec(cmd, function(returnCode,stdout,stderr) {
			if (returnCode == 0) {
 				return JSON.parse(stdout);
 			} else {
 				//TODO Handle error if returnCode != 0
 			}
 		});
    };
})();

var json = {
	"com.windows.microsoft.magnifier": {
		"settings": {
			"Magnification": 250, 
			"Inver": 1
		}, 
		"options": {
			"hKey": "HKEY_CURRENT_USER", 
			"path": "Software\\Microsoft\\ScreenMagnifier"
		}
	}
};
gpii.settingsHandlers.windowsRegistrySettingsHandler.set(json);
