#pragma once

#ifndef S__IDBSETTINGITEM_H
#define S__IDBSETTINGITEM_H

#include "BaseSettingItem.h"

struct DbSettingItem : public BaseSettingItem {
    using BaseSettingItem::BaseSettingItem;

    DbSettingItem();
};

#endif // S__IDBSETTINGITEM_H