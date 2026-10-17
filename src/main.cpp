#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <algorithm>
#include <vector>

using namespace geode::prelude;

namespace {
    void showMessage(char const* title, char const* message) {
        FLAlertLayer::create(title, message, "OK")->show();
    }

    bool isSpike(int id) {
        return id == 8 || id == 39 || id == 103;
    }

    bool isSlope(int id) {
        switch (id) {
            case 289: case 291: case 294: case 295: case 296: case 297:
            case 305: case 307: case 349: case 351:
            case 363: case 364: case 366: case 367:
            case 371: case 372: case 373: case 374:
            case 513: case 514: case 522: case 523: case 531: case 532:
            case 603: case 604: case 612: case 613: case 621: case 622:
            case 630: case 631: case 639: case 640: case 665: case 666:
            case 681: case 682: case 683: case 684: case 685: case 686:
            case 687: case 688: case 689: case 690: case 691: case 692:
            case 693: case 694: case 695: case 696: case 697: case 698:
            case 699: case 700: case 701: case 702: case 703: case 704:
            case 705: case 706: case 707: case 708: case 713: case 714:
            case 715: case 716: case 730: case 731: case 732: case 733:
            case 762: case 763: case 771: case 772: case 826: case 827:
            case 828: case 829: case 960: case 961: case 969: case 970:
            case 1014: case 1015: case 1033: case 1034: case 1037: case 1038:
            case 1041: case 1042: case 1043: case 1044:
            case 1091: case 1092: case 1107: case 1108:
            case 1187: case 1188: case 1198: case 1199:
            case 1206: case 1207: case 1224: case 1225:
                return true;
            default:
                return false;
        }
    }

    enum class Shape { Block, Slope, WideSlope, Spike };

    Shape getShape(GameObject* object) {
        auto id = object ? object->m_objectID : 1;
        if (isSpike(id)) return Shape::Spike;
        if (isSlope(id)) {
            auto size = object->getContentSize();
            return size.width >= size.height * 1.65f ? Shape::WideSlope : Shape::Slope;
        }
        return Shape::Block;
    }

    class WhiteShapeNode : public CCNode {
        CCDrawNode* m_draw = nullptr;

    public:
        static WhiteShapeNode* create(GameObject* source) {
            auto ret = new WhiteShapeNode();
            if (ret && ret->initWithSource(source)) {
                ret->autorelease();
                return ret;
            }
            CC_SAFE_DELETE(ret);
            return nullptr;
        }

        bool initWithSource(GameObject* source) {
            if (!CCNode::init() || !source) return false;

            auto size = source->getContentSize();
            if (size.width < 1.f) size.width = 30.f;
            if (size.height < 1.f) size.height = 30.f;

            m_draw = CCDrawNode::create();
            if (!m_draw) return false;
            this->addChild(m_draw);

            ccColor4F white = {1.f, 1.f, 1.f, 1.f};
            ccColor4F transparent = {1.f, 1.f, 1.f, 0.f};
            float halfW = size.width * 0.5f;
            float halfH = size.height * 0.5f;
            CCPoint points[4];

            switch (getShape(source)) {
                case Shape::Spike:
                    points[0] = CCPointMake(-halfW, -halfH);
                    points[1] = CCPointMake( halfW, -halfH);
                    points[2] = CCPointMake(0.f, halfH);
                    m_draw->drawPolygon(points, 3, white, 0.f, transparent);
                    break;
                case Shape::Slope:
                case Shape::WideSlope:
                    points[0] = CCPointMake(-halfW, -halfH);
                    points[1] = CCPointMake( halfW, -halfH);
                    points[2] = CCPointMake( halfW, halfH);
                    m_draw->drawPolygon(points, 3, white, 0.f, transparent);
                    break;
                default:
                    m_draw->drawRect({-halfW, -halfH}, {halfW, halfH}, white, 0.f, transparent);
                    break;
            }

            this->setContentSize(size);
            this->setAnchorPoint({0.5f, 0.5f});
            this->setPosition(source->getPosition());
            this->setRotation(source->getRotation());
            this->setScaleX(source->getScaleX());
            this->setScaleY(source->getScaleY());
            return true;
        }
    };

    // The two editor buttons can be dragged together. A normal tap still activates the button.
    class DraggableMenu : public CCMenu {
        CCPoint m_lastTouch = CCPointZero;
        bool m_dragging = false;

    public:
        static DraggableMenu* create() {
            auto ret = new DraggableMenu();
            if (ret && ret->init()) {
                ret->autorelease();
                return ret;
            }
            CC_SAFE_DELETE(ret);
            return nullptr;
        }

        bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
            if (!CCMenu::ccTouchBegan(touch, event)) return false;
            m_lastTouch = touch->getLocation();
            m_dragging = false;
            return true;
        }

        void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
            auto now = touch->getLocation();
            auto delta = now - m_lastTouch;
            if (delta.x != 0.f || delta.y != 0.f) {
                m_dragging = true;
                this->setPosition(this->getPosition() + delta);
            }
            m_lastTouch = now;
            CCMenu::ccTouchMoved(touch, event);
        }

        void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
            CCMenu::ccTouchEnded(touch, event);
            m_dragging = false;
        }

        void ccTouchCancelled(CCTouch* touch, CCEvent* event) override {
            CCMenu::ccTouchCancelled(touch, event);
            m_dragging = false;
        }
    };
}

class $modify(WhiteStructurePreviewEditorUI, EditorUI) {
    struct Fields {
        DraggableMenu* menu = nullptr;
        CCMenuItemSpriteExtra* previewButton = nullptr;
        CCMenuItemSpriteExtra* confirmButton = nullptr;
        std::vector<GameObject*> selected;
        std::vector<WhiteShapeNode*> previews;
        bool previewVisible = false;
    };

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        auto menu = DraggableMenu::create();
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
                if (object && std::find(result.begin(), result.end(), object) == result.end()) result.push_back(object);
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
            showMessage("White Structure Preview", "Select one or more blocks, ramps or spikes first.");
            return;
        }

        m_fields->selected = selected;

        // Important: previews are children of the object layer, not the editor UI.
        // This keeps them in the same coordinate/camera space as the selected objects.
        for (auto object : selected) {
            if (!object || !object->getParent()) continue;
            auto preview = WhiteShapeNode::create(object);
            if (!preview) continue;
            auto parent = object->getParent();
            parent->addChild(preview, 100000);
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
        if (!m_fields->previewVisible || m_fields->previews.empty()) {
            showMessage("White Structure Preview", "Press PREVIEW first.");
            return;
        }

        // Confirmation only closes the visual preview. It does not modify or duplicate
        // the original selected Geometry Dash objects.
        clearPreview();
    }
};
