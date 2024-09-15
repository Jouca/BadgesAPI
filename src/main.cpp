#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/CommentCell.hpp>
#include <Geode/modify/CCMenu.hpp>

// Badges API
// Mod API made by Jouca

#include <Geode/loader/Log.hpp>
#include <string>
#include "badges/BadgeMenu.h"
#include "RescalingNode.hpp"

class $modify(CustomCommentCell, CommentCell) {
	struct Fields {
		bool loaded = false;
		bool plus_badge = false;
		int badgeCount = 0;
		BadgeMenu* badgeMenu = nullptr;
    };

	static void onModify(auto& self) {
        self.setHookPriority("CommentCell::loadFromComment", INT_MIN);
    }

	void updateBadges(CCArray* childsToRemove, CCMenu* username_menu, CCArray* badges) {
		// Remove all badges from the layer
		CCObject* childObj;
		CommentCell* cell = this;
    	CCARRAY_FOREACH(childsToRemove, childObj) as<CCNode*>(childObj)->removeFromParent();

		m_fields->badgeCount = badges->count();

		// If there is only one badge, let it on the layer
		if (badges->count() == 1) {
			m_fields->plus_badge = false;

			CCNode* child = as<CCNode*>(badges->objectAtIndex(0));

			if (this->getChildByIDRecursive("percentage-label")) {
				username_menu->insertBefore(child, cell->getChildByIDRecursive("percentage-label"));
			} else {
				username_menu->addChild(child);
			}
		}

		// If there are more than one badge, create the badge menu
		if (badges->count() >= 2) {
			CCSprite* child_spr;
			auto child = as<CCMenuItemSpriteExtra*>(badges->objectAtIndex(0));
			if (child) {
				CCSprite* sprite = as<CCSprite*>(child->getNormalImage());
				if (sprite) {
					child_spr = CCSprite::createWithSpriteFrame(sprite->displayFrame());
					child_spr->setScale(as<CCSprite*>(child->getChildren()->objectAtIndex(0))->getScale());
				}
			} else {
				child_spr = CCSprite::createWithSpriteFrame(as<CCSprite*>(badges->objectAtIndex(0))->displayFrame());
				child_spr->setScale(as<CCSprite*>(badges->objectAtIndex(0))->getScale());
			}

			CCSprite* badge_plus = CCSprite::create("plusLittleBtn.png"_spr);
			badge_plus->setID("badgeAPI-plus-btn-spr");
			CCNode* badge_plus_node = RescalingNode::create(badge_plus, { child_spr->getContentSize().width * 0.4f, child_spr->getContentSize().height * 0.4f });
			badge_plus_node->setPosition({child_spr->getContentSize().width * 0.7f, 0.f});
			if (child_spr) child_spr->addChild(badge_plus_node);

			auto badge_plus_item = CCMenuItemSpriteExtra::create(child_spr, this, menu_selector(CustomCommentCell::onBadgePlus));
			badge_plus_item->setUserObject(badges);
			badge_plus_item->setID("badgeAPI-plus-badge");
			badge_plus_item->setPosition(CCPoint { -5.f, -1.f });

			CCMenuItemSpriteExtra* badge_api_plus = typeinfo_cast<CCMenuItemSpriteExtra*>(this->getChildByIDRecursive("badgeAPI-plus-badge"));
			if (badge_api_plus) {
				badge_api_plus->removeFromParent();
			}

			if (cell->getChildByIDRecursive("percentage-label") != nullptr) {
				username_menu->insertBefore(badge_plus_item, cell->getChildByIDRecursive("percentage-label"));
			} else {
				username_menu->addChild(badge_plus_item);
			}

			username_menu->updateLayout();

			m_fields->plus_badge = true;
		}
	}

	void updateBadgesSchedule(float dt) {
		auto layer = m_mainLayer;
		int badge_count = m_fields->badgeCount;

		CCMenu* username_menu = typeinfo_cast<CCMenu*>(layer->getChildByIDRecursive("username-menu"));
		if (!username_menu) {
			log::error("Could not find username-menu");
			return;
		}

		if (!m_fields->loaded) {
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

			// Check if badges are already loaded on BadgeMenu and then remove those on the profile page
			CCObject* childObj;
			CCARRAY_FOREACH(badge_api_plus_item, childObj) {
				CCNode* child = as<CCNode*>(childObj);
				if (auto user_child = username_menu->getChildByIDRecursive(child->getID())) {
					user_child->removeFromParent();
					username_menu->updateLayout();
				}
			}
			
			CCArray* temp2 = CCArray::create();
			CCARRAY_FOREACH(badge_api_plus_item, childObj) {
				CCNode* child = as<CCNode*>(childObj);
				temp2->addObject(child);
			}

			CCARRAY_FOREACH(temp, childObj) {
				CCNode* child = as<CCNode*>(childObj);

				// check if ID is present on badge_api_plus_item objects
				bool found = false;
				CCObject* childObj2;
				CCARRAY_FOREACH(badge_api_plus_item, childObj2) {
					CCNode* child2 = as<CCNode*>(childObj2);
					if (child->getID() == child2->getID()) {
						found = true;
						break;
					}
				}
				if (!found) temp2->addObject(child);
			}

			temp = temp2;
		}

		if (m_fields->badgeCount < temp->count()) {
			updateBadges(childsToRemoveTemp, username_menu, temp);
		}
	}

	void onBadgePlus(CCObject* pSender) {
		auto childs = as<CCArray*>(static_cast<CCNode*>(pSender)->getUserObject());

		// rescale the badges
		CCArray* childsRescaled = CCArray::create();
		for (int i = 0; i < childs->count(); i++) {
			auto child = as<CCMenuItemSpriteExtra*>(childs->objectAtIndex(i));
			if (child) {
				CCSprite* sprite = as<CCSprite*>(child->getNormalImage());
				if (sprite) {
					sprite->setScale(sprite->getScale() * 1.5f);
				}
				child->updateSprite();
			} else {
				CCSprite* sprite = as<CCSprite*>(childs->objectAtIndex(i));
				sprite->setScale(sprite->getScale() * 1.5f);
			}

			childsRescaled->addObject(child);
		}

		m_fields->badgeMenu = BadgeMenu::scene(childsRescaled);
	}
	
	void loadFromComment(GJComment* p0) {
		if (this->getChildByIDRecursive("mod-badge")) as<CCNode*>(this->getChildByIDRecursive("mod-badge"))->removeFromParent();

		CommentCell::loadFromComment(p0);

		if (m_fields->loaded) {
			return;
		}

		m_fields->loaded = true;

		auto layer = m_mainLayer;

		CCMenu* username_menu = typeinfo_cast<CCMenu*>(layer->getChildByIDRecursive("username-menu"));
		if (!username_menu) {
			//log::error("Could not find username-menu");
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

		updateBadges(childsToRemove, username_menu, childs);

		this->schedule(schedule_selector(CustomCommentCell::updateBadgesSchedule), 0.0f);
	}
};

class $modify(CustomProfilePage, ProfilePage) {
	struct Fields {
		bool loaded = false;
		bool plus_badge = false;
		int badgeCount = 0;
		BadgeMenu* badgeMenu = nullptr;
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
			CCSprite* child_spr;
			auto child = as<CCMenuItemSpriteExtra*>(badges->objectAtIndex(0));
			if (child) {
				CCSprite* sprite = as<CCSprite*>(child->getNormalImage());
				if (sprite) {
					child_spr = CCSprite::createWithSpriteFrame(sprite->displayFrame());
					child_spr->setScale(as<CCSprite*>(child->getChildren()->objectAtIndex(0))->getScale());
				}
			} else {
				child_spr = CCSprite::createWithSpriteFrame(as<CCSprite*>(badges->objectAtIndex(0))->displayFrame());
				child_spr->setScale(as<CCSprite*>(badges->objectAtIndex(0))->getScale());
			}

			CCSprite* badge_plus = CCSprite::create("plusLittleBtn.png"_spr);
			badge_plus->setID("badgeAPI-plus-btn-spr");
			CCNode* badge_plus_node = RescalingNode::create(badge_plus, { child_spr->getContentSize().width * 0.4f, child_spr->getContentSize().height * 0.4f });
			badge_plus_node->setPosition({child_spr->getContentSize().width * 0.7f, 0.f});
			if (child_spr) child_spr->addChild(badge_plus_node);

			auto badge_plus_item = CCMenuItemSpriteExtra::create(child_spr, this, menu_selector(CustomProfilePage::onBadgePlus));
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
		int badge_count = m_fields->badgeCount;

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

			// Check if badges are already loaded on BadgeMenu and then remove those on the profile page
			CCObject* childObj;
			CCARRAY_FOREACH(badge_api_plus_item, childObj) {
				CCNode* child = as<CCNode*>(childObj);
				if (auto user_child = username_menu->getChildByIDRecursive(child->getID())) {
					user_child->removeFromParent();
					username_menu->updateLayout();
				}
			}
			
			CCArray* temp2 = CCArray::create();
			CCARRAY_FOREACH(badge_api_plus_item, childObj) {
				CCNode* child = as<CCNode*>(childObj);
				temp2->addObject(child);
			}

			CCARRAY_FOREACH(temp, childObj) {
				CCNode* child = as<CCNode*>(childObj);

				// check if ID is present on badge_api_plus_item objects
				bool found = false;
				CCObject* childObj2;
				CCARRAY_FOREACH(badge_api_plus_item, childObj2) {
					CCNode* child2 = as<CCNode*>(childObj2);
					if (child->getID() == child2->getID()) {
						found = true;
						break;
					}
				}
				if (!found) temp2->addObject(child);
			}

			temp = temp2;
		}

		if (m_fields->badgeCount < temp->count()) {
			updateBadges(childsToRemoveTemp, username_menu, label, temp);
		}
	}

	void onBadgePlus(CCObject* pSender) {
		auto childs = as<CCArray*>(static_cast<CCNode*>(pSender)->getUserObject());
		m_fields->badgeMenu = BadgeMenu::scene(childs);
	}

	void loadPageFromUserInfo(GJUserScore* a2) {
		if (this->getChildByIDRecursive("mod-badge")) as<CCNode*>(this->getChildByIDRecursive("mod-badge"))->removeFromParent();

		ProfilePage::loadPageFromUserInfo(a2);

		if (m_fields->loaded) {
			return;
		}

		m_fields->loaded = true;

		auto layer = m_mainLayer;

		CCMenu* username_menu = typeinfo_cast<CCMenu*>(layer->getChildByIDRecursive("username-menu"));
		if (!username_menu) {
			//log::error("Could not find username-menu");
			return;
		}
		CCLabelBMFont* label = typeinfo_cast<CCLabelBMFont*>(layer->getChildByIDRecursive("username-label"));
		if (!label) {
			//log::error("Could not find username-label");
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