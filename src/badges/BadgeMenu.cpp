#include "BadgeMenu.h"
#include "BadgeMenuCell.h"

void BadgeMenu::scene(CCArray* array) {
    auto popup = new BadgeMenu(array);

    if (popup && popup->init(300.0f, 210.0f)) {
        popup->autorelease();
        popup->setZOrder(1000);
        CCDirector::sharedDirector()->getRunningScene()->addChild(popup);
    } else {
        CC_SAFE_DELETE(popup);
    }
};

BadgeMenu::BadgeMenu(CCArray* data) : m_data(data) {}

bool BadgeMenu::setup() {
    auto winSize = cocos2d::CCDirector ::sharedDirector()->getWinSize();
    auto director = cocos2d::CCDirector::sharedDirector();

    registerWithTouchDispatcher();
    setTouchEnabled(true);
    setKeypadEnabled(true);

    CCArrayExt<BadgeMenuCell*> cells;
    int badges_max = 8;

    CCObject* obj;
    CCArray* cell = CCArray::create();
    CCARRAY_FOREACH(m_data, obj) {
        cell->addObject(as<CCNode*>(obj));
        if (cell->count() == badges_max) {
            cells.push_back(BadgeMenuCell::create(cell, { 300, 210 }));
            cell = CCArray::create();
        }
    }
    cells.push_back(BadgeMenuCell::create(cell, { 300, 210 }));

    auto border = OldBorder::create(ListView::create(cells.inner(), 40, 292, 198), { 0xBF, 0x72, 0x3E, 0xFF }, {294, 198}, {1, 1});
    border->setPosition(m_mainLayer->getContentSize() / 2 - border->getContentSize() / 2);
    setTouchPriority(100);
    m_mainLayer->addChild(border);

    return true;
}