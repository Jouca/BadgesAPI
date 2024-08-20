#pragma once
#include "includes.h"

#define __GETTER(type, name, capital_name) type get##capital_name() const { return m_##name; }

#define GETTER(type, name, capital_name) \
        public: __GETTER(type, name, capital_name) \
        private: type m_##name

class RescalingNode : public CCNode {
public:
    static RescalingNode* create(CCNode* node, const CCSize& size);

    RescalingNode(CCNode* node);
    CCNode* getNode() { return m_node; }
    bool init(const CCSize& size);
    void setRescaleWidth(const bool value);
    void setRescaleHeight(const bool value);
    void setContentSize(const CCSize& size) override;
private:
    CCNode* m_node;
    GETTER(CCNode*, m_node, Node);
    GETTER(bool, rescaleWidth, RescaleWidth);
    GETTER(bool, rescaleHeight, RescaleHeight);
};