#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/CCMenu.hpp>

// Badges API
// Mod API made by Jouca

#include <Geode/loader/Log.hpp>
#include <string>
#include "badges/BadgeMenu.h"

class $modify(CustomProfilePage, ProfilePage) {
	struct Fields {
		bool loaded = false;
		bool plus_badge = false;
		int badgeCount = 0;
    };

	static void onModify(auto& self) {
        self.setHookPriority("ProfilePage::loadPageFromUserInfo", INT_MIN);
    }

	void updateBadges(CCArray* childsToRemove, CCMenu* username_menu, CCLabelBMFont* label, CCArray* badges) {
		// Remove all badges from the layer
		CCObject* childObj;
    	CCARRAY_FOREACH(childsToRemove, childObj) as<CCNode*>(childObj)->removeFromParent();

		m_fields->badgeCount = badges->count();

		// If there is only one badge, let it on the layer
		if (badges->count() == 1) {
			m_fields->plus_badge = false;

			CCNode* child = as<CCNode*>(badges->objectAtIndex(0));
			username_menu->addChild(child);
		}

		// If there are more than one badge, create the badge menu
		if (badges->count() >= 2) {
			CCSprite* badge_plus = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
			badge_plus->setScale(.5f);
			auto badge_plus_item = CCMenuItemSpriteExtra::create(badge_plus, this, menu_selector(CustomProfilePage::onBadgePlus));
			badge_plus_item->setUserObject(badges);
			badge_plus_item->setID("badgeAPI-plus-badge");
			badge_plus_item->setPosition(label->getPosition() + CCPoint { -5.f, -1.f });

			CCMenuItemSpriteExtra* badge_api_plus = typeinfo_cast<CCMenuItemSpriteExtra*>(this->getChildByIDRecursive("badgeAPI-plus-badge"));
			if (badge_api_plus) {
				badge_api_plus->removeFromParent();
			}	
			username_menu->addChild(badge_plus_item);

			username_menu->updateLayout();

			m_fields->plus_badge = true;
		}
	}

	void updateBadgesSchedule(float dt) {
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

		if (!m_fields->loaded) {
			return;
		}

		if (m_fields->badgeCount < 1 && !m_fields->plus_badge) {
			return;
		}

		CCArray* childsToRemoveTemp = CCArray::create();
		CCArray* temp = CCArray::create();
		CCObject* childObj;
    	CCARRAY_FOREACH(username_menu->getChildren(), childObj) {
			CCNode* child = as<CCNode*>(childObj);
			std::string find_str ("-badge");
			std::string child_id = child->getID();
			std::transform(child_id.begin(), child_id.end(), child_id.begin(), [](unsigned char c){ return std::tolower(c); });
			if (child_id.find(find_str) != std::string::npos) {
				if (child_id != "badgeapi-plus-badge") {
					childsToRemoveTemp->addObject(child);
					temp->addObject(child);
				}
			}
		}

		CCMenuItemSpriteExtra* badge_api_plus = typeinfo_cast<CCMenuItemSpriteExtra*>(this->getChildByIDRecursive("badgeAPI-plus-badge"));
		if (badge_api_plus) {
			auto badge_api_plus_item = as<CCArray*>(static_cast<CCNode*>(badge_api_plus)->getUserObject());
			CCArray* temp2 = CCArray::create();
			CCObject* childObj;
			CCARRAY_FOREACH(badge_api_plus_item, childObj) {
				CCNode* child = as<CCNode*>(childObj);
				temp2->addObject(child);
			}

			CCARRAY_FOREACH(temp, childObj) {
				CCNode* child = as<CCNode*>(childObj);
				temp2->addObject(child);
			}

			temp = temp2;
		}

		if (m_fields->badgeCount < temp->count()) {
			updateBadges(childsToRemoveTemp, username_menu, label, temp);
		}
	}

	void onBadgePlus(CCObject* pSender) {
		auto childs = as<CCArray*>(static_cast<CCNode*>(pSender)->getUserObject());
		BadgeMenu::scene(childs);
	}

	void loadPageFromUserInfo(GJUserScore* a2) {
		ProfilePage::loadPageFromUserInfo(a2);

		if (m_fields->loaded) {
			return;
		}
		m_fields->loaded = true;

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

		CCArray* childs = CCArray::create();
		CCArray* childsToRemove = CCArray::create();
    	CCObject* childObj;
    	CCARRAY_FOREACH(username_menu->getChildren(), childObj) {
			CCNode* child = as<CCNode*>(childObj);
			std::string find_str ("-badge");
			if (child->getID().find(find_str) != std::string::npos) {
				childs->addObject(child);
				childsToRemove->addObject(child);
			}
		}

		updateBadges(childsToRemove, username_menu, label, childs);

		this->schedule(schedule_selector(CustomProfilePage::updateBadgesSchedule), 0.0f);
	}
};