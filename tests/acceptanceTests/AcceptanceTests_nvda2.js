/*

GPII Acceptance Testing

Copyright 2014 Raising the Floor International

Licensed under the New BSD license. You may not use this file except in
compliance with this License.

You may obtain a copy of the License at
https://github.com/gpii/universal/LICENSE.txt
*/

/*global require,process*/

"use strict";
var fluid = require("universal"),
    path = require("path"),
    gpii = fluid.registerNamespace("gpii");

fluid.require("./AcceptanceTests_include", require);

var testDefs = [
    {
        name: "Testing NVDA using a set of basic common terms",
        token: "screenreader_common",
        settingsHandlers: {
            "gpii.settingsHandlers.INISettingsHandler": {
                "org.nvda-project": [
                    {
                        "settings": {
                            "speech.symbolLevel": "300",
                            "speech.espeak.rate": 17.20430107526882,
                            "speech.espeak.pitch": 15,
                            "speech.espeak.voice": "en\\en-wi",
                            "speech.espeak.volume": 75,
                            "reviewCursor.followFocus": false,
                            "reviewCursor.followCaret": true,
                            "reviewCursor.followMouse": true,
                            "keyboard.speakTypedWords": true,
                            "speech.espeak.rateBoost": true,
                            "keyboard.speakTypedCharacters": false,
                            "presentation.reportHelpBalloons": false,
                            "speech.espeak.sayCapForCapitals": true,
                            "virtualBuffers.autoSayAllOnPageLoad": false
                        },
                        "options": {
                            "path": "C:\\Users\\kasper\\AppData\\Roaming\\nvda\\nvda.ini",
                            "allowNumberSignComments": true,
                            "allowSubSections": true
                        }
                    }
                ]
            }
        },
        processes: [
            {
                "command": "tasklist /fi \"STATUS eq RUNNING\" /FI \"IMAGENAME eq nvda.exe\" | find /I \"nvda.exe\" /C",
                "expectConfigured": "1",
                "expectRestored": "0"
            }
        ]
    }
];

gpii.acceptanceTesting.windows.runTests("nvda2_config", testDefs);