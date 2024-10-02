#include "BadgeMenuCell.h"

BadgeMenuCell* BadgeMenuCell::create(CCArray* badges, const CCSize& size) {
    BadgeMenuCell* instance = new BadgeMenuCell(badges, size);

    if (instance && instance->init(size)) {
        return instance;
    } else {
        CC_SAFE_DELETE(instance);

        return nullptr;
    }
}

BadgeMenuCell::BadgeMenuCell(CCArray* badges, const CCSize& size) : GenericListCell("", size), m_badges(badges) {}

bool BadgeMenuCell::init(const CCSize& size) {
    CCMenu* menu = CCMenu::create();
    menu->setLayout(
        RowLayout::create()->setCrossAxisAlignment(AxisAlignment::Center)->setGap(10)
    );
    menu->setPosition(size.width / 2, 20);
    menu->setContentSize({292, 40});

    for (int i = 0; i < m_badges->count(); i++) {
        auto badge = typeinfo_cast<CCNode*>(m_badges->objectAtIndex(i));
        menu->addChild(badge);
    }

    menu->updateLayout();

    addChild(menu);

    return true;
}