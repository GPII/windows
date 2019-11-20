/**
 * Additional type IIDs found during research.
 *
 * Copyright 2019 Raising the Floor - US
 *
 * Licensed under the New BSD license. You may not use this file except in
 * compliance with this License.
 *
 * You may obtain a copy of the License at
 * https://github.com/GPII/universal/blob/master/LICENSE.txt
 */

#pragma once

#include <combaseapi.h>

/// <summary>
///
/// </summary>
IID IID_ISettingItem {
    0x40c037cc,
    0xd8bf,
    0x489e,
    {
        0x86,
        0x97,
        0xd6,
        0x6b,
        0xaa,
        0x32,
        0x21,
        0xbf
    }
};
/// <summary>
///
/// </summary>
IID IID_ISettingItem2 {
    0x32572999,
    0x399d,
    0x40ff,
    {
       0xa4,
       0x23,
       0x35,
       0x95,
       0xb4,
       0xdc,
       0x99,
       0x3b
   }
};

// +6: SystemSettings::DataModel::CSettingBase::OnSettingsAppSuspending
// +7: SystemSettings::DataModel::CSettingBase::OnSettingsAppResuming
IID IID_ISettingsAppNotification {
    0xdff43ddc,
    0x7d1d,
    0x4899,
    {
        0x94,
        0xc5,
        0xa2,
        0xa0,
        0x6e,
        0x17,
        0x77,
        0x12
    }
};

// +6: SystemSettings::DataModel::CSettingBase::GetPageSessionId(struct* _GUID)
// +7: SystemSettings::DataModel::CSettingBase::SetPageSessionId(struct _GUID)
IID IID_ISettingTelemetry {
    0x7cf1e617,
    0xfe1e,
    0x48ef,
    {
        0x89,
        0x62,
        0x40,
        0xa4,
        0x25,
        0xd2,
        0x1d,
        0x7e
    }
};

/// <summary>
///
/// </summary>
IID IID_IWeakReferenceSource {
    0x00000038,
    0x0000,
    0x0000,
    {
        0xc0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x46
    }
};

/// <summary>
///
/// </summary>
IID IID_ISettingObservableVector {
    0xc5267a7c,
    0x640e,
    0x5cfc,
    {
        0xa7,
        0xcc,
        0x40,
        0x1c,
        0xcf,
        0x42,
        0x6e,
        0x36
    }
};
