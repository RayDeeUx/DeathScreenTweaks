#include <Geode/modify/EffectGameObject.hpp>
#include "Manager.hpp"

using namespace geode::prelude;

class $modify(MyEffectGameObject, EffectGameObject) {
    void triggerObject(GJBaseGameLayer* gjbgl, int p1, gd::vector<int> const* p2) {
        EffectGameObject::triggerObject(gjbgl, p1, p2);
        if (this->m_objectType != GameObjectType::UserCoin && this->m_objectType != GameObjectType::SecretCoin) return;
        std::map<GameObject*, bool>& coinsMap = Manager::getSharedInstance()->coins;
        if (coinsMap.contains(this)) {
            coinsMap.at(this) = true;
            // log::info("collected coin number {}", static_cast<int>(std::distance(coinsMap.begin(), coinsMap.find(this))) + 1);
        }
    }
};