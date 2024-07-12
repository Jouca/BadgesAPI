#pragma once
#include "../includes.h"
#include "../OldBorder.hpp"

class BadgeMenu : public Popup<> {
    protected:
        bool setup() override;
    public:
        CCArray* m_data;

        BadgeMenu(CCArray* data);
        static BadgeMenu* scene(CCArray* array);
};