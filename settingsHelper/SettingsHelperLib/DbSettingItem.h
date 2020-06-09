/**
 * DBSettingItem - a wrapper for ISettingItem representing settings
 * inside a DynamicSettingsDatabase.
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

#include "BaseSettingItem.h"

struct DbSettingItem : public BaseSettingItem {
    using BaseSettingItem::BaseSettingItem;

    DbSettingItem();
};
