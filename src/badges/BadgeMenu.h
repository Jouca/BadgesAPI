#pragma once
#include "../includes.h"

class BadgeMenu : public Popup<> {
    protected:
        bool setup() override;
        cocos2d::CCSize m_sScrLayerSize;
    public:
        static void scene(CCArray* array);
};