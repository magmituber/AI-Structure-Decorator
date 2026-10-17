#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <algorithm>
#include <vector>

using namespace geode::prelude;

namespace {
    void showMessage(char const* title, char const* message) {
        FLAlertLayer::create(title, message, "OK")->show();
    }
}

class $modify(WhiteStructurePreviewEditorUI, EditorUI) {
    struct Fields {
        CCMenu* menu = nullptr;
        CCMenuItemSpriteExtra* previewButton = nullptr;
        CCMenuItemSpriteExtra* confirmButton = nullptr;
        std::vector<GameObject*> selected;
        std::vector<CCLayerColor*> previews;
        bool previewVisible = false;
    };

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        auto menu = CCMenu::create();
        if (!menu) return true;
        menu->setID("white-preview-menu"_spr);
        menu->setPosition({0.f, 0.f});

        auto previewLabel = CCLabelBMFont::create("PREVIEW", "bigFont.fnt");
        auto confirmLabel = CCLabelBMFont::create("CONFIRM", "bigFont.fnt");
        if (!previewLabel || !confirmLabel) return true;
        previewLabel->setScale(0.35f);
        confirmLabel->setScale(0.35f);

        auto preview = CCMenuItemSpriteExtra::create(
            previewLabel, this, menu_selector(WhiteStructurePreviewEditorUI::onPreview)
        );
        auto confirm = CCMenuItemSpriteExtra::create(
            confirmLabel, this, menu_selector(WhiteStructurePreviewEditorUI::onConfirm)
        );
        if (!preview || !confirm) return true;

        auto size = CCDirector::sharedDirector()->getWinSize();
        float centerX = size.width * 0.5f;
        float topY = size.height - 30.f;
        preview->setPosition({centerX - 48.f, topY});
        confirm->setPosition({centerX + 48.f, topY});
        preview->setID("white-preview-button"_spr);
        confirm->setID("white-confirm-button"_spr);
        confirm->setVisible(false);

        menu->addChild(preview);
        menu->addChild(confirm);
        this->addChild(menu, 1000);

        m_fields->menu = menu;
        m_fields->previewButton = preview;
        m_fields->confirmButton = confirm;
        return true;
    }

    void clearPreview() {
        for (auto* preview : m_fields->previews) {
            if (preview) preview->removeFromParentAndCleanup(true);
        }
        m_fields->previews.clear();
        m_fields->selected.clear();
        m_fields->previewVisible = false;
        if (m_fields->confirmButton) m_fields->confirmButton->setVisible(false);
    }

    std::vector<GameObject*> getSelection() {
        std::vector<GameObject*> result;
        if (m_selectedObject) result.push_back(m_selectedObject);
        if (m_selectedObjects) {
            for (auto object : geode::cocos::CCArrayExt<GameObject, false>(m_selectedObjects)) {
                if (object && std::find(result.begin(), result.end(), object) == result.end()) {
                    result.push_back(object);
                }
            }
        }
        return result;
    }

    void onPreview(CCObject*) {
        clearPreview();
        if (!m_editorLayer) {
            showMessage("White Structure Preview", "Open a level in the editor first.");
            return;
        }

        auto selected = getSelection();
        if (selected.empty()) {
            showMessage("White Structure Preview", "Select one or more blocks, ramps or other objects first.");
            return;
        }

        m_fields->selected = selected;

        for (auto object : selected) {
            if (!object) continue;
            auto size = object->getContentSize();
            if (size.width < 1.f) size.width = 30.f;
            if (size.height < 1.f) size.height = 30.f;

            auto preview = CCLayerColor::create({255, 255, 255, 220}, size.width, size.height);
            if (!preview) continue;
            preview->setAnchorPoint({0.5f, 0.5f});
            preview->setPosition(object->getPosition());
            preview->setRotation(object->getRotation());
            preview->setScaleX(object->getScaleX());
            preview->setScaleY(object->getScaleY());
            m_editorLayer->addChild(preview, 999);
            m_fields->previews.push_back(preview);
        }

        if (m_fields->previews.empty()) {
            clearPreview();
            showMessage("White Structure Preview", "The selected objects could not be previewed.");
            return;
        }

        m_fields->previewVisible = true;
        m_fields->confirmButton->setVisible(true);
    }

    void onConfirm(CCObject*) {
        if (!m_fields->previewVisible || m_fields->selected.empty() || !m_editorLayer) {
            showMessage("White Structure Preview", "Press PREVIEW first.");
            return;
        }

        int created = 0;
        for (auto source : m_fields->selected) {
            if (!source) continue;

            auto object = this->createObject(1, source->getPosition());
            if (!object) continue;

            object->setRotation(source->getRotation());
            object->setScaleX(source->getScaleX());
            object->setScaleY(source->getScaleY());
            object->setObjectColor({255, 255, 255});
            object->setOpacity(255);
            ++created;
        }

        clearPreview();
        if (created > 0) {
            this->updateButtons();
            this->updateObjectInfoLabel();
            showMessage("White Structure Preview", "Preview confirmed: white blocks were placed in the editor.");
        } else {
            showMessage("White Structure Preview", "No blocks could be created.");
        }
    }
};
