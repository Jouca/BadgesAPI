#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>

// Badges API
// Mod API made by Jouca

#include <Geode/loader/Log.hpp>
#include <string>

#include "badges/BadgeMenu.h"

class $modify(CustomProfilePage, ProfilePage) {
	static void onModify(auto& self) {
        self.setHookPriority("ProfilePage::loadPageFromUserInfo", INT_MIN);
    }

	struct Fields {
        CCArray* m_childs = CCArray::create();
    };

	void onBadgePlus(CCObject* pSender) {
		auto childs = as<CCArray*>(static_cast<CCNode*>(pSender)->getUserObject());
		BadgeMenu::scene(childs);
	}

	void loadPageFromUserInfo(GJUserScore* a2) {
		ProfilePage::loadPageFromUserInfo(a2);

		auto layer = m_mainLayer;

		CCMenu* username_menu = typeinfo_cast<CCMenu*>(layer->getChildByIDRecursive("username-menu"));
		if (!username_menu) {
			log::error("Could not find username-menu");
			return;
		}
		CCLabelBMFont* label = typeinfo_cast<CCLabelBMFont*>(layer->getChildByIDRecursive("username-label"));
		if (!label) {
			log::error("Could not find username-label");
			return;
		}

		CCArray* childsToRemove = CCArray::create();
    	CCObject* childObj;
    	CCARRAY_FOREACH(username_menu->getChildren(), childObj) {
			CCNode* child = as<CCNode*>(childObj);
			std::string find_str ("-badge");
			if (child->getID().find(find_str) != std::string::npos) {
				m_fields->m_childs->addObject(child);
				childsToRemove->addObject(child);
			}
		}

		// Remove all badges from the layer
    	CCARRAY_FOREACH(childsToRemove, childObj) as<CCNode*>(childObj)->removeFromParent();

		// If there is only one badge, let it on the layer
		if (m_fields->m_childs->count() == 1) {
			CCNode* child = as<CCNode*>(m_fields->m_childs->objectAtIndex(0));
			username_menu->addChild(child);
		}

		// If there are more than one badge, create the badge menu
		if (m_fields->m_childs->count() >= 2) {
			CCSprite* badge_plus = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
			badge_plus->setScale(.5f);
			auto badge_plus_item = CCMenuItemSpriteExtra::create(badge_plus, this, menu_selector(CustomProfilePage::onBadgePlus));
			badge_plus_item->setUserObject(m_fields->m_childs);
			badge_plus_item->setPosition(label->getPosition() + CCPoint { -5.f, -1.f });
			username_menu->addChild(badge_plus_item);

			username_menu->updateLayout();
		}
	}
};