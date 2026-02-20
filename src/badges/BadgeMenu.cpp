#include "BadgeMenu.h"
#include "BadgeMenuCell.h"

BadgeMenu* BadgeMenu::scene(CCArray* array) {
    auto popup = new BadgeMenu(array);

    if (popup && popup->init()) {
        popup->autorelease();
        popup->setZOrder(1000);
        popup->show();
    } else {
        CC_SAFE_DELETE(popup);
    }

    return popup;
};

BadgeMenu::BadgeMenu(CCArray* data) : m_data(data) {}

bool BadgeMenu::init() {
    if (!geode::Popup::init(300.0f, 210.0f)) return;

    auto winSize = cocos2d::CCDirector ::sharedDirector()->getWinSize();
    auto director = cocos2d::CCDirector::sharedDirector();

    m_noElasticity = true;

    CCArrayExt<BadgeMenuCell*> cells;
    int badges_max = 8;

    CCObject* obj;
    CCArray* cell = CCArray::create();
    // CCARRAY_FOREACH(m_data, obj) {
    //     cell->addObject(obj);
    //     if (cell->count() == badges_max) {
    //         cells.push_back(BadgeMenuCell::create(cell, { 300, 210 }));
    //         cell = CCArray::create();
    //     }
    // }
    for (CCObject* obj : CCArrayExt<CCObject*>(m_data)) {
        if (!obj) continue;
        cell->addObject(obj);
        if (cell->count() == badges_max) {
            cells.push_back(BadgeMenuCell::create(cell, { 300, 210 }));
            cell = CCArray::create();
        }
    }
    cells.push_back(BadgeMenuCell::create(cell, { 300, 210 }));

    auto border = OldBorder::create(ListView::create(cells.inner(), 40, 292, 160), { 0xBF, 0x72, 0x3E, 0xFF }, {294, 162}, {1, 1});
    border->ignoreAnchorPointForPosition(false);
    border->setPositionX(m_mainLayer->getContentSize().width / 2);
    border->setPositionY(border->getContentSize().height / 2 + 5);
    m_mainLayer->addChild(border);

    CCSprite* title_badges = CCSprite::create("badges.png"_spr);
    title_badges->setPosition({m_mainLayer->getContentSize().width / 2, 186});
    title_badges->setScale(0.625f);
    m_mainLayer->addChild(title_badges);

    return true;
}