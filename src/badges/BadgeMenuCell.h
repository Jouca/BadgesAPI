#pragma once
#include "../includes.h"

struct BadgeMenuCell : public GenericListCell {
    static BadgeMenuCell* create(CCArray* badges, const CCSize& size);
private:
    CCArray* m_badges;

    BadgeMenuCell(CCArray* badges, const CCSize& size);
    bool init(const CCSize& size);
};