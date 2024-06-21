#include "BadgeMenu.h"

void BadgeMenu::scene(CCArray* array) {
    auto popup = new BadgeMenu();

    if (popup && popup->init(300.0f, 210.0f)) {
        popup->autorelease();
        popup->setZOrder(1000);
        CCDirector::sharedDirector()->getRunningScene()->addChild(popup);
    } else {
        CC_SAFE_DELETE(popup);
    }
};

bool BadgeMenu::setup() {
    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
    auto director = cocos2d::CCDirector::sharedDirector();

    registerWithTouchDispatcher();
    setTouchEnabled(true);
    setKeypadEnabled(true);

    return true;
}