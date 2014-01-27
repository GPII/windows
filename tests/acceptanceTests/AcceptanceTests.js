/*

GPII Acceptance Testing

Copyright 2013 Raising the Floor International

Licensed under the New BSD license. You may not use this file except in
compliance with this License.

You may obtain a copy of the License at
https://github.com/gpii/universal/LICENSE.txt
*/

/*global __dirname, require*/

"use strict";
var fluid = require("universal"),
    path = require("path"),
    kettle = fluid.registerNamespace("kettle"),
    gpii = fluid.registerNamespace("gpii");

fluid.require("../gpii/node_modules/registrySettingsHandler", require);
fluid.require("../gpii/node_modules/registryResolver", require);
fluid.require("../gpii/node_modules/spiSettingsHandler", require);

fluid.require("universal/tests/AcceptanceTests", require);

var configPath = path.resolve(__dirname, "./acceptanceTests/setup1/configs");
var gpiiConfig = {
   nodeEnv: "development-config",
   configPath: configPath
};

var testDefs = [
    {
        name: "Testing os_win7 using Flat matchmaker",
        config: gpiiConfig,
        token: "os_win7",
        settingsHandlers: {
            "gpii.windows.spiSettingsHandler": {
                "data": [
                    {
                        "settings": {
                            "MouseTrails": {
                                "path": {
                                    "get": "pvParam",
                                    "set": "uiParam"
                                },
                                "value": 10
                            }
                        },
                        "options": {
                            "getAction": "SPI_GETMOUSETRAILS",
                            "setAction": "SPI_SETMOUSETRAILS",
                            "uiParam": 0,
                            "pvParam": {
                                "type": "BOOL"
                            }
                        }
                    }, { //high contrast settings
                        "settings": {
                            "HighContrastOn": {
                                "path": "pvParam.dwFlags.HCF_HIGHCONTRASTON",
                                "value": true
                            }
                        },
                        "options": {
                            "getAction": "SPI_GETHIGHCONTRAST",
                            "setAction": "SPI_SETHIGHCONTRAST",
                            "uiParam": "struct_size",
                            "pvParam": {
                                "type": "struct",
                                "name": "HIGHCONTRAST"
                            }
                        }
                    }
                ]
           },
            "gpii.windows.registrySettingsHandler": {
                "data": [{ //magnifier stuff
                    "settings": {
                        "Invert": {
                            "dataType": "REG_DWORD",
                            "value": 1
                        },
                        "Magnification": {
                            "dataType": "REG_DWORD",
                            "value": 150
                        },
                        "MagnificationMode": {
                            "dataType": "REG_DWORD",
                            "value": 3
                        },
                        "FollowFocus": {
                            "value": 0,
                            "dataType": "REG_DWORD"
                        },
                        "FollowCaret": {
                            "value": 1,
                            "dataType": "REG_DWORD"
                        },
                        "FollowMouse": {
                            "value": 1,
                            "dataType": "REG_DWORD"
                        }
                    },
                    "options": {
                        "hKey": "HKEY_CURRENT_USER",
                        "path": "Software\\Microsoft\\ScreenMagnifier"
                    }
                }, { //cursor size stuff
                    "settings": {
                        "No": {
                            "value": "%SystemRoot%\\cursors\\aero_unavail_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "Hand": {
                            "value": "%SystemRoot%\\cursors\\aero_link_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "Help": {
                            "value": "%SystemRoot%\\cursors\\aero_helpsel_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "Wait": {
                            "value": "%SystemRoot%\\cursors\\aero_busy_xl.ani",
                            "dataType": "REG_SZ"
                        },
                        "Arrow": {
                            "value": "%SystemRoot%\\cursors\\aero_arrow_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "NWPen": {
                            "value": "%SystemRoot%\\cursors\\aero_pen_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeNS": {
                            "value": "%SystemRoot%\\cursors\\aero_ns_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeWE": {
                            "value": "%SystemRoot%\\cursors\\aero_ew_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeAll": {
                            "value": "%SystemRoot%\\cursors\\aero_move_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "UpArrow": {
                            "value": "%SystemRoot%\\cursors\\aero_up_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeNESW": {
                            "value": "%SystemRoot%\\cursors\\aero_nesw_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeNWSE": {
                            "value": "%SystemRoot%\\cursors\\aero_nwse_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "AppStarting": {
                            "value": "%SystemRoot%\\cursors\\aero_working_xl.ani",
                            "dataType": "REG_SZ"
                        }
                    },
                    "options": {
                        "hKey": "HKEY_CURRENT_USER",
                        "path": "Control Panel\\Cursors"
                    }
                }]
            }
        },
        processes: [
            {
                "command": "tasklist /fi \"STATUS eq RUNNING\" /FI \"IMAGENAME eq Magnify.exe\" | find /I \"Magnify.exe\" /C",
                "expectConfigured": "1",
                "expectRestored": "0"
            }
        ]
    }, {
        name: "Testing os_common using Flat matchmaker",
        config: gpiiConfig,
        token: "os_common",
        settingsHandlers: {
            "gpii.windows.spiSettingsHandler": {
                "data": [
                    {
                        "settings": {
                            "MouseTrails": {
                                "path": {
                                    "get": "pvParam",
                                    "set": "uiParam"
                                },
                                "value": 10
                            }
                        },
                        "options": {
                            "getAction": "SPI_GETMOUSETRAILS",
                            "setAction": "SPI_SETMOUSETRAILS",
                            "uiParam": 0,
                            "pvParam": {
                                "type": "BOOL"
                            }
                        }
                    }, { //high contrast settings
                        "settings": {
                            "HighContrastOn": {
                                "path": "pvParam.dwFlags.HCF_HIGHCONTRASTON",
                                "value": true
                            }
                        },
                        "options": {
                            "getAction": "SPI_GETHIGHCONTRAST",
                            "setAction": "SPI_SETHIGHCONTRAST",
                            "uiParam": "struct_size",
                            "pvParam": {
                                "type": "struct",
                                "name": "HIGHCONTRAST"
                            }
                        }
                    }
                ]
           },
            "gpii.windows.registrySettingsHandler": {
                "data": [{ //magnifier stuff
                    "settings": {
                        "Invert": {
                            "dataType": "REG_DWORD",
                            "value": 1
                        },
                        "Magnification": {
                            "dataType": "REG_DWORD",
                            "value": 150
                        },
                        "MagnificationMode": {
                            "dataType": "REG_DWORD",
                            "value": 3
                        },
                        "FollowFocus": {
                            "value": 0,
                            "dataType": "REG_DWORD"
                        },
                        "FollowCaret": {
                            "value": 1,
                            "dataType": "REG_DWORD"
                        },
                        "FollowMouse": {
                            "value": 1,
                            "dataType": "REG_DWORD"
                        }
                    },
                    "options": {
                        "hKey": "HKEY_CURRENT_USER",
                        "path": "Software\\Microsoft\\ScreenMagnifier"
                    }
                }, { //cursor size stuff
                    "settings": {
                        "No": {
                            "value": "%SystemRoot%\\cursors\\aero_unavail_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "Hand": {
                            "value": "%SystemRoot%\\cursors\\aero_link_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "Help": {
                            "value": "%SystemRoot%\\cursors\\aero_helpsel_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "Wait": {
                            "value": "%SystemRoot%\\cursors\\aero_busy_xl.ani",
                            "dataType": "REG_SZ"
                        },
                        "Arrow": {
                            "value": "%SystemRoot%\\cursors\\aero_arrow_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "NWPen": {
                            "value": "%SystemRoot%\\cursors\\aero_pen_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeNS": {
                            "value": "%SystemRoot%\\cursors\\aero_ns_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeWE": {
                            "value": "%SystemRoot%\\cursors\\aero_ew_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeAll": {
                            "value": "%SystemRoot%\\cursors\\aero_move_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "UpArrow": {
                            "value": "%SystemRoot%\\cursors\\aero_up_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeNESW": {
                            "value": "%SystemRoot%\\cursors\\aero_nesw_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeNWSE": {
                            "value": "%SystemRoot%\\cursors\\aero_nwse_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "AppStarting": {
                            "value": "%SystemRoot%\\cursors\\aero_working_xl.ani",
                            "dataType": "REG_SZ"
                        }
                    },
                    "options": {
                        "hKey": "HKEY_CURRENT_USER",
                        "path": "Control Panel\\Cursors"
                    }
                }]
            }
        },
        processes: [
            {
                "command": "tasklist /fi \"STATUS eq RUNNING\" /FI \"IMAGENAME eq Magnify.exe\" | find /I \"Magnify.exe\" /C",
                "expectConfigured": "1",
                "expectRestored": "0"
            }
        ]
    }, {
        name: "Testing os_gnome using Flat matchmaker",
        config: gpiiConfig,
        token: "os_gnome",
        settingsHandlers: {
            "gpii.windows.registrySettingsHandler": {
                "data": [{ //magnifier stuff
                    "settings": {
                        "Magnification": {
                            "dataType": "REG_DWORD",
                            "value": 150
                        },
                        "MagnificationMode": {
                            "dataType": "REG_DWORD",
                            "value": 2
                        }
                    },
                    "options": {
                        "hKey": "HKEY_CURRENT_USER",
                        "path": "Software\\Microsoft\\ScreenMagnifier"
                    }
                }, { //cursor size stuff
                    "settings": {
                        "No": {
                            "value": "%SystemRoot%\\cursors\\aero_unavail_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "Hand": {
                            "value": "%SystemRoot%\\cursors\\aero_link_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "Help": {
                            "value": "%SystemRoot%\\cursors\\aero_helpsel_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "Wait": {
                            "value": "%SystemRoot%\\cursors\\aero_busy_xl.ani",
                            "dataType": "REG_SZ"
                        },
                        "Arrow": {
                            "value": "%SystemRoot%\\cursors\\aero_arrow_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "NWPen": {
                            "value": "%SystemRoot%\\cursors\\aero_pen_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeNS": {
                            "value": "%SystemRoot%\\cursors\\aero_ns_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeWE": {
                            "value": "%SystemRoot%\\cursors\\aero_ew_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeAll": {
                            "value": "%SystemRoot%\\cursors\\aero_move_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "UpArrow": {
                            "value": "%SystemRoot%\\cursors\\aero_up_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeNESW": {
                            "value": "%SystemRoot%\\cursors\\aero_nesw_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "SizeNWSE": {
                            "value": "%SystemRoot%\\cursors\\aero_nwse_xl.cur",
                            "dataType": "REG_SZ"
                        },
                        "AppStarting": {
                            "value": "%SystemRoot%\\cursors\\aero_working_xl.ani",
                            "dataType": "REG_SZ"
                        }
                    },
                    "options": {
                        "hKey": "HKEY_CURRENT_USER",
                        "path": "Control Panel\\Cursors"
                    }
                }]
            }
        },
        processes: [
            {
                "command": "tasklist /fi \"STATUS eq RUNNING\" /FI \"IMAGENAME eq Magnify.exe\" | find /I \"Magnify.exe\" /C",
                "expectConfigured": "1",
                "expectRestored": "0"
            }
        ]
    }
];

testDefs = gpii.acceptanceTesting.buildTests(testDefs);
module.exports = kettle.tests.bootstrap(testDefs);
