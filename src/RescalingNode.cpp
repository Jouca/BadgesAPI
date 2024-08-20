#include "RescalingNode.hpp"

RescalingNode* RescalingNode::create(CCNode* node, const CCSize& size) {
    RescalingNode* instance = new RescalingNode(node);

    if (instance && instance->init(size)) {
        instance->autorelease();

        return instance;
    } else {
        CC_SAFE_DELETE(instance);

        return nullptr;
    }
}

RescalingNode::RescalingNode(CCNode* node) : m_node(node), m_rescaleWidth(true), m_rescaleHeight(true) { }

bool RescalingNode::init(const CCSize& size) {
    if (!CCNode::init()) return false;

    m_node->setAnchorPoint({ 0.5f, 0.5f });
    m_node->ignoreAnchorPointForPosition(false);
    m_node->setPosition(size / 2);

    this->addChild(m_node);
    this->setContentSize(size);

    return true;
}

void RescalingNode::setRescaleWidth(const bool value) {
    m_rescaleWidth = value;

    this->setContentSize(this->getContentSize());
}

void RescalingNode::setRescaleHeight(const bool value) {
    m_rescaleHeight = value;

    this->setContentSize(this->getContentSize());
}

void RescalingNode::setContentSize(const CCSize& size) {
    CCNode::setContentSize(size);

    m_node->setPosition(size / 2);

    if (m_rescaleWidth && m_rescaleHeight) {
        m_node->setScale(size.width / m_node->getContentWidth(), size.height / m_node->getContentHeight());
    } else if (m_rescaleWidth) {
        m_node->setScale(size.width / m_node->getContentWidth());
    } else if (m_rescaleHeight) {
        m_node->setScale(size.height / m_node->getContentHeight());
    }
}