#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "AutoDecoPanel.hpp"
using namespace geode::prelude;
class $modify(AutoDecoEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editor) {
        if (!EditorUI::init(editor)) return false;
        if (auto panel=autodeco::Panel::create(this)) { panel->setID("panel"_spr); addChild(panel,1000); }
        return true;
    }
};
