#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/CommentCell.hpp>
#include <Geode/modify/CCMenu.hpp>
#include <Geode/loader/Log.hpp>

// Badges API
// Mod API made by Jouca

#include <string>
#include "badges/BadgeMenu.h"
#include "RescalingNode.hpp"

// Helper: Extracts priority from badge ID (format: ...-badge:{number}), returns 9999 if not found
static int getBadgePriority(CCNode* badge) {
	std::string id = geode::utils::string::toLower(badge->getID());
	if (!geode::utils::string::contains(id, "-badge:")) return 9999;
	// std::transform(id.begin(), id.end(), id.begin(), [](unsigned char c){ return std::tolower(c); });
	size_t pos = id.find("-badge:");
	if (pos != std::string::npos) {
		size_t start = pos + 7;
		size_t end = id.find_first_not_of("0123456789", start);
		std::string num = id.substr(start, end == std::string::npos ? end : end - start);
		// try {
		// 	int val = std::stoi(num);
		// 	return val > 0 ? val : 9999;
		// } catch (...) {}
		return geode::utils::numFromString<int>(num).unwrapOr(9999);
	}
	return 9999;
}

// Helper: Sorts a CCArray* of badges by priority (lowest first)
static CCArray* sortBadgesByPriority(CCArray* badges) {
	std::vector<CCNode*> badgeVec;
	// for (int i = 0; i < badges->count(); ++i) {
	// 	badgeVec.push_back(static_cast<CCNode*>(badges->objectAtIndex(i)));
	// }
	for (CCNode* badge : CCArrayExt<CCNode*>(badges)) {
		if (!badge) continue;
		badgeVec.push_back(badge);
	}
	std::sort(badgeVec.begin(), badgeVec.end(), [](CCNode* a, CCNode* b) {
		return getBadgePriority(a) < getBadgePriority(b);
	});
	CCArray* sorted = CCArray::create();
	for (auto* b : badgeVec) sorted->addObject(b);
	return sorted;
}

class $modify(CustomCommentCell, CommentCell) {
	struct Fields {
		bool loaded = false;
		bool plus_badge = false;
		bool pressed = false;
		int badgeCount = 0;
		BadgeMenu* badgeMenu = nullptr;
	};

	static void onModify(auto& self) {
		(void)self.setHookPriority("CommentCell::loadFromComment", INT_MIN);
	}

	void updateBadges(CCArray* childsToRemove, CCMenu* username_menu, CCArray* badges) {
		// Remove all badges from the layer
		// CCObject* childObj;
		// CCARRAY_FOREACH(childsToRemove, childObj) static_cast<CCNode*>(childObj)->removeFromParent();
		CommentCell* cell = this;
		for (CCNode* childObj : CCArrayExt<CCNode*>(childsToRemove)) {
			if (!childObj) continue;
			childObj->removeFromParent();
		}

		// Sort badges by priority before displaying
		CCArray* sortedBadges = sortBadgesByPriority(badges);
		m_fields->badgeCount = sortedBadges->count();

		// determine which badge to select as the first and foremost badge
		CCSprite* posterBoyBadge = nullptr;
		for (CCNode* badge : CCArrayExt<CCNode*>(sortedBadges)) {
			if (!badge || (!typeinfo_cast<CCSprite*>(badge) && !typeinfo_cast<CCMenuItemSpriteExtra*>(badge))) continue;
			posterBoyBadge = static_cast<CCSprite*>(badge);
			break;
		}
		if (!posterBoyBadge) {
			posterBoyBadge = CCSprite::createWithSpriteFrameName("GJ_profileButton_001.png"); // fallback
		}

		// If there is only one badge, let it on the layer
		if (sortedBadges->count() == 1) {
			m_fields->plus_badge = false;
			CCNode* child = static_cast<CCNode*>(posterBoyBadge);
			if (this->getChildByIDRecursive("percentage-label")) {
				username_menu->insertBefore(child, cell->getChildByIDRecursive("percentage-label"));
			} else {
				username_menu->addChild(child);
			}
		}

		// If there are more than one badge, create the badge menu
		if (sortedBadges->count() > 1) {
			CCSprite* child_spr;
			auto child = typeinfo_cast<CCMenuItemSpriteExtra*>(posterBoyBadge);
			if (child) {
				CCSprite* sprite = typeinfo_cast<CCSprite*>(child->getNormalImage());
				if (sprite) {
					child_spr = CCSprite::createWithSpriteFrame(sprite->displayFrame());
					child_spr->setScale(child->getNormalImage()->getScale());
				}
			} else {
				child_spr = CCSprite::createWithSpriteFrame(static_cast<CCSprite*>(posterBoyBadge)->displayFrame());
				child_spr->setScale(static_cast<CCSprite*>(posterBoyBadge)->getScale());
			}

			CCSprite* badge_plus = CCSprite::create("plusLittleBtn.png"_spr);
			badge_plus->setID("badgeAPI-plus-btn-spr");
			CCNode* badge_plus_node = RescalingNode::create(badge_plus, { child_spr->getContentSize().width * 0.4f, child_spr->getContentSize().height * 0.4f });
			badge_plus_node->setPosition({child_spr->getContentSize().width * 0.7f, 0.f});
			if (child_spr) child_spr->addChild(badge_plus_node);

			auto badge_plus_item = CCMenuItemSpriteExtra::create(child_spr, this, menu_selector(CustomCommentCell::onBadgePlus));
			badge_plus_item->setUserObject(sortedBadges);
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
		// CCARRAY_FOREACH(username_menu->getChildren(), childObj) {
		// CCNode* child = static_cast<CCNode*>(childObj);
		// std::string find_str ("-badge");
		// std::string child_id = child->getID();
		// std::transform(child_id.begin(), child_id.end(), child_id.begin(), [](unsigned char c){ return std::tolower(c); });
		// if (child_id.find(find_str) != std::string::npos) {
		// 	if (child_id != "badgeapi-plus-badge") {
		// 		childsToRemoveTemp->addObject(child);
		// 		temp->addObject(child);
		// 	}
		// }
		// }
		for (CCNode* child : username_menu->getChildrenExt()) {
			if (!child) continue;
			std::string child_id = geode::utils::string::toLower(child->getID());
            if (child_id == "badgeapi-plus-badge") continue;
            if (!geode::utils::string::contains(child_id, "-badge")) continue;
			childsToRemoveTemp->addObject(child);
			temp->addObject(child);
		}

		CCMenuItemSpriteExtra* badge_api_plus = typeinfo_cast<CCMenuItemSpriteExtra*>(this->getChildByIDRecursive("badgeAPI-plus-badge"));
		if (badge_api_plus) {
			auto badge_api_plus_item = static_cast<CCArray*>(static_cast<CCNode*>(badge_api_plus)->getUserObject());

			// Check if badges are already loaded on BadgeMenu and then remove those on the profile page
			// CCObject* childObj;
			// CCARRAY_FOREACH(badge_api_plus_item, childObj) {
			// 	CCNode* child = static_cast<CCNode*>(childObj);
			// 	if (auto user_child = username_menu->getChildByIDRecursive(child->getID())) {
			// 		user_child->removeFromParent();
			// 		username_menu->updateLayout();
			// 	}
			// }
			for (CCNode* child : CCArrayExt<CCNode*>(badge_api_plus_item)) {
				if (!child) continue;
				auto user_child = username_menu->getChildByIDRecursive(child->getID());
				if (!user_child) continue;
				user_child->removeFromParent();
			}
			username_menu->updateLayout(); // update layout once outside of the loop
			
			CCArray* temp2 = CCArray::create();
			// CCARRAY_FOREACH(badge_api_plus_item, childObj) {
			// 	temp2->addObject(childObj);
			// }
			for (CCNode* child : CCArrayExt<CCNode*>(badge_api_plus_item)) {
				if (!child) continue;
				temp2->addObject(child);
			}

			// CCARRAY_FOREACH(temp, childObj) {
			// 	CCNode* child = static_cast<CCNode*>(childObj);

			// 	// check if ID is present on badge_api_plus_item objects
			// 	bool found = false;
			// 	CCObject* childObj2;
			// 	CCARRAY_FOREACH(badge_api_plus_item, childObj2) {
			// 		CCNode* child2 = static_cast<CCNode*>(childObj2);
			// 		if (child->getID() == child2->getID()) {
			// 			found = true;
			// 			break;
			// 		}
			// 	}
			// 	if (!found) temp2->addObject(child);
			// }
			for (CCNode* child : CCArrayExt<CCNode*>(temp)) {
				if (!child) continue;
				bool found = false;
				for (CCNode* child2 : CCArrayExt<CCNode*>(badge_api_plus_item)) {
					if (!child2) continue;
					if (child->getID() != child2->getID()) continue;
					found = true;
					break;
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
		auto childs = static_cast<CCArray*>(static_cast<CCNode*>(pSender)->getUserObject());
		// Sort by priority before showing in menu
		CCArray* sortedBadges = sortBadgesByPriority(childs);
		// rescale the badges
		CCArray* childsRescaled = CCArray::create();
		for (int i = 0; i < sortedBadges->count(); i++) {
			auto child = typeinfo_cast<CCMenuItemSpriteExtra*>(sortedBadges->objectAtIndex(i));
			if (child) {
				CCMenuItemSpriteExtra* new_child = CCMenuItemSpriteExtra::create(child->getNormalImage(), this, child->m_pfnSelector);
				CCSprite* sprite = CCSprite::createWithSpriteFrame(static_cast<CCSprite*>(new_child->getNormalImage())->displayFrame());
				if (sprite) {
					sprite->setScale(child->getNormalImage()->getScale() * 1.5f);
				}
				new_child->setID(child->getID());
				new_child->setTag(child->getTag());
				new_child->setNormalImage(sprite);
				new_child->setUserObject(child->getUserObject());
				new_child->updateSprite();
				childsRescaled->addObject(new_child);
			} else if (typeinfo_cast<CCSprite*>(sortedBadges->objectAtIndex(i))) {
				CCSprite* sprite = static_cast<CCSprite*>(sortedBadges->objectAtIndex(i));
				CCSprite* sprite2 = CCSprite::createWithSpriteFrame(sprite->displayFrame());
				sprite2->setScale(sprite2->getScale() * 1.5f);
				childsRescaled->addObject(sprite2);
			}
		}
		m_fields->badgeMenu = BadgeMenu::scene(childsRescaled);
	}
	
	void loadFromComment(GJComment* p0) {
		if (this->getChildByIDRecursive("mod-badge")) this->getChildByIDRecursive("mod-badge")->removeFromParent();

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
		(void)self.setHookPriority("ProfilePage::loadPageFromUserInfo", INT_MIN);
	}

	void updateBadges(CCArray* childsToRemove, CCMenu* username_menu, CCLabelBMFont* label, CCArray* badges) {
		// Remove all badges from the layer
		CCObject* childObj;
		// CCARRAY_FOREACH(childsToRemove, childObj) static_cast<CCNode*>(childObj)->removeFromParent();
		for (CCNode* child : CCArrayExt<CCNode*>(childsToRemove)) {
			if (!child) continue;
			child->removeFromParent();
		}

		// Sort badges by priority before displaying
		CCArray* sortedBadges = sortBadgesByPriority(badges);
		m_fields->badgeCount = sortedBadges->count();

		// determine which badge to select as the first and foremost badge
		CCSprite* posterBoyBadge = nullptr;
		for (CCNode* badge : CCArrayExt<CCNode*>(sortedBadges)) {
			if (!badge || (!typeinfo_cast<CCSprite*>(badge) && !typeinfo_cast<CCMenuItemSpriteExtra*>(badge))) continue;
			posterBoyBadge = static_cast<CCSprite*>(badge);
			break;
		}
		if (!posterBoyBadge) {
			posterBoyBadge = CCSprite::createWithSpriteFrameName("GJ_profileButton_001.png"); // fallback
		}

		// If there is only one badge, let it on the layer
		if (sortedBadges->count() == 1) {
			m_fields->plus_badge = false;
			CCNode* child = typeinfo_cast<CCNode*>(posterBoyBadge);
			username_menu->addChild(child);
		}

		// If there are more than one badge, create the badge menu
		if (sortedBadges->count() > 1) {
			CCSprite* child_spr;
			auto child = typeinfo_cast<CCMenuItemSpriteExtra*>(posterBoyBadge);
			if (child) {
				CCSprite* sprite = typeinfo_cast<CCSprite*>(child->getNormalImage());
				if (sprite) {
					child_spr = CCSprite::createWithSpriteFrame(sprite->displayFrame());
					child_spr->setScale(child->getNormalImage()->getScale());
				}
			} else if (typeinfo_cast<CCSprite*>(posterBoyBadge)) {
				child_spr = CCSprite::createWithSpriteFrame(static_cast<CCSprite*>(posterBoyBadge)->displayFrame());
				child_spr->setScale(static_cast<CCSprite*>(posterBoyBadge)->getScale());
			}

			CCSprite* badge_plus = CCSprite::create("plusLittleBtn.png"_spr);
			badge_plus->setID("badgeAPI-plus-btn-spr");
			CCNode* badge_plus_node = RescalingNode::create(badge_plus, { child_spr->getContentSize().width * 0.4f, child_spr->getContentSize().height * 0.4f });
			badge_plus_node->setPosition({child_spr->getContentSize().width * 0.7f, 0.f});
			if (child_spr) child_spr->addChild(badge_plus_node);

			auto badge_plus_item = CCMenuItemSpriteExtra::create(child_spr, this, menu_selector(CustomProfilePage::onBadgePlus));
			badge_plus_item->setUserObject(sortedBadges);
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
		// CCARRAY_FOREACH(username_menu->getChildren(), childObj) {
		// 	CCNode* child = static_cast<CCNode*>(childObj);
		// 	std::string find_str ("-badge");
		// 	std::string child_id = child->getID();
		// 	std::transform(child_id.begin(), child_id.end(), child_id.begin(), [](unsigned char c){ return std::tolower(c); });
		// 	if (child_id.find(find_str) != std::string::npos) {
		// 		if (child_id != "badgeapi-plus-badge") {
		// 			childsToRemoveTemp->addObject(child);
		// 			temp->addObject(child);
		// 		}
		// 	}
		// }
		for (CCNode* child : username_menu->getChildrenExt()) {
			if (!child) continue;
			std::string child_id = geode::utils::string::toLower(child->getID());
        	if (child_id == "badgeapi-plus-badge") continue;
            if (!geode::utils::string::contains(child_id, "-badge")) continue;
			childsToRemoveTemp->addObject(child);
			temp->addObject(child);
		}

		CCMenuItemSpriteExtra* badge_api_plus = typeinfo_cast<CCMenuItemSpriteExtra*>(this->getChildByIDRecursive("badgeAPI-plus-badge"));
		if (badge_api_plus) {
			auto badge_api_plus_item = static_cast<CCArray*>(static_cast<CCNode*>(badge_api_plus)->getUserObject());

			// Check if badges are already loaded on BadgeMenu and then remove those on the profile page
			// CCObject* childObj;
			// CCARRAY_FOREACH(badge_api_plus_item, childObj) {
			// 	CCNode* child = static_cast<CCNode*>(childObj);
			// 	if (auto user_child = username_menu->getChildByIDRecursive(child->getID())) {
			// 		user_child->removeFromParent();
			// 		username_menu->updateLayout();
			// 	}
			// }
			for (CCNode* child : CCArrayExt<CCNode*>(badge_api_plus_item)) {
				if (!child) continue;
				auto user_child = username_menu->getChildByIDRecursive(child->getID());
				if (!user_child) continue;
				user_child->removeFromParent();
			}
			username_menu->updateLayout(); // update layout once after loop finishes
			
			CCArray* temp2 = CCArray::create();
			// CCARRAY_FOREACH(badge_api_plus_item, childObj) {
			// 	temp2->addObject(childObj);
			// }
			for (CCNode* node : CCArrayExt<CCNode*>(badge_api_plus_item)) {
				if (!node) continue;
				temp2->addObject(node);
			}

			// CCARRAY_FOREACH(temp, childObj) {
			// 	CCNode* child = static_cast<CCNode*>(childObj);

			// 	// check if ID is present on badge_api_plus_item objects
			// 	bool found = false;
			// 	CCObject* childObj2;
			// 	CCARRAY_FOREACH(badge_api_plus_item, childObj2) {
			// 		CCNode* child2 = static_cast<CCNode*>(childObj2);
			// 		if (child->getID() == child2->getID()) {
			// 			found = true;
			// 			break;
			// 		}
			// 	}
			// 	if (!found) temp2->addObject(child);
			// }
			for (CCNode* child : CCArrayExt<CCNode*>(temp)) {
				if (!child) continue;
				bool found = false;
				for (CCNode* child2 : CCArrayExt<CCNode*>(badge_api_plus_item)) {
					if (!child2) continue;
					if (child->getID() != child2->getID()) continue;
					found = true;
					break;
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
		auto childs = static_cast<CCArray*>(static_cast<CCNode*>(pSender)->getUserObject());
		// Sort by priority before showing in menu
		CCArray* sortedBadges = sortBadgesByPriority(childs);
		m_fields->badgeMenu = BadgeMenu::scene(sortedBadges);
	}

	void loadPageFromUserInfo(GJUserScore* a2) {
		if (this->getChildByIDRecursive("mod-badge")) this->getChildByIDRecursive("mod-badge")->removeFromParent();

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

		this->schedule(schedule_selector(CustomProfilePage::updateBadgesSchedule), 0.0f);
	}
};
