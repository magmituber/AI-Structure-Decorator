#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <algorithm>
#include <cmath>
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
        // Base slopes and the large set of 1.8+ slope variants.
        switch (id) {
            case 6: case 7:
            case 289: case 291: case 294: case 295: case 296: case 297:
            case 299: case 301: case 305: case 307: case 309: case 311:
            case 315: case 317: case 321: case 323: case 325: case 327:
            case 328: case 329: case 331: case 333: case 337: case 339:
            case 343: case 345: case 349: case 351: case 353: case 355:
            case 358: case 363: case 364: case 366: case 367:
            case 371: case 372: case 373: case 374:
            case 473: case 474: case 475: case 483: case 484: case 492: case 493:
            case 513: case 514: case 522: case 523: case 531: case 532:
            case 603: case 604: case 612: case 613: case 621: case 622:
            case 630: case 631: case 639: case 640: case 651: case 652:
            case 665: case 666:
            case 681: case 682: case 683: case 684: case 685: case 686:
            case 687: case 688: case 689: case 690: case 691: case 692:
            case 693: case 694: case 695: case 696: case 697: case 698:
            case 699: case 700: case 701: case 702: case 703: case 704:
            case 705: case 706: case 707: case 708: case 709: case 710:
            case 711: case 712: case 713: case 714: case 715: case 716:
            case 726: case 727: case 728: case 729: case 730: case 731:
            case 732: case 733: case 762: case 763: case 771: case 772:
            case 826: case 827: case 828: case 829:
            case 886: case 887: case 960: case 961: case 969: case 970:
            case 1014: case 1015: case 1033: case 1034: case 1037: case 1038:
            case 1041: case 1042: case 1043: case 1044:
            case 1091: case 1092: case 1107: case 1108:
            case 1187: case 1188: case 1198: case 1199:
            case 1206: case 1207: case 1224: case 1225:
            // Relisted old outline slopes.
            case 1338: case 1339:
            case 1743: case 1744: case 1745: case 1746: case 1747:
            case 1748: case 1749: case 1750:
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
        GameObject* m_source = nullptr;
    public:
        static WhiteShapeNode* create(GameObject* source) {
            auto ret = new WhiteShapeNode();
            if (ret && ret->initWithSource(source)) { ret->autorelease(); return ret; }
            CC_SAFE_DELETE(ret);
            return nullptr;
        }

        bool initWithSource(GameObject* source) {
            if (!CCNode::init() || !source) return false;
            m_source = source;
            auto size = source->getContentSize();
            if (size.width < 1.f) size.width = 30.f;
            if (size.height < 1.f) size.height = 30.f;
            m_draw = CCDrawNode::create();
            if (!m_draw) return false;
            this->addChild(m_draw);

            ccColor4F white = {1.f, 1.f, 1.f, 1.f};
            ccColor4F transparent = {1.f, 1.f, 1.f, 0.f};
            constexpr float outlineWidth = 2.5f;
            float halfW = size.width * 0.5f;
            float halfH = size.height * 0.5f;
            CCPoint points[4];

            switch (getShape(source)) {
                case Shape::Spike:
                    points[0] = CCPointMake(-halfW, -halfH);
                    points[1] = CCPointMake(halfW, -halfH);
                    points[2] = CCPointMake(0.f, halfH);
                    m_draw->drawPolygon(points, 3, transparent, outlineWidth, white);
                    break;
                case Shape::Slope:
                case Shape::WideSlope:
                    points[0] = CCPointMake(-halfW, -halfH);
                    points[1] = CCPointMake(halfW, -halfH);
                    points[2] = CCPointMake(halfW, halfH);
                    m_draw->drawPolygon(points, 3, transparent, outlineWidth, white);
                    break;
                default:
                    m_draw->drawRect({-halfW, -halfH}, {halfW, halfH}, transparent, outlineWidth, white);
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

        bool syncToLayer(CCNode* layer) {
            if (!layer || !m_source) return false;
            auto sourceParent = m_source->getParent();
            if (!sourceParent) return false;

            // The preview itself lives on the editor UI layer to avoid CCSpriteBatchNode
            // crashes. Recalculate its screen position every frame from the source object's
            // current world position, so camera panning/zooming affects it exactly like the
            // real object instead of leaving it glued to the screen.
            auto worldPosition = sourceParent->convertToWorldSpace(m_source->getPosition());
            this->setPosition(layer->convertToNodeSpace(worldPosition));
            this->setRotation(m_source->getRotation());
            this->setScaleX(m_source->getScaleX());
            this->setScaleY(m_source->getScaleY());
            return true;
        }
    };

    class DraggableMenu : public CCMenu {
        CCMenuItemSpriteExtra* m_pressed = nullptr;
        CCPoint m_startTouch = CCPointZero;
        CCPoint m_startPosition = CCPointZero;
        bool m_dragging = false;
    public:
        static DraggableMenu* create() {
            auto ret = new DraggableMenu();
            if (ret && ret->init()) { ret->autorelease(); return ret; }
            CC_SAFE_DELETE(ret);
            return nullptr;
        }
        bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
            auto point = this->convertTouchToNodeSpace(touch);
            m_pressed = nullptr;
            for (auto child : CCArrayExt<CCNode*, false>(this->getChildren())) {
                auto item = typeinfo_cast<CCMenuItemSpriteExtra*>(child);
                if (item && item->isVisible() && item->isEnabled() && item->boundingBox().containsPoint(point)) {
                    m_pressed = item;
                    break;
                }
            }
            if (!m_pressed) return false;
            auto parent = this->getParent();
            if (!parent) {
                m_pressed = nullptr;
                return false;
            }
            m_startTouch = parent->convertToNodeSpace(touch->getLocation());
            m_startPosition = this->getPosition();
            m_dragging = false;
            return true;
        }
        void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
            if (!m_pressed) return;
            auto parent = this->getParent();
            if (!parent) {
                m_pressed = nullptr;
                m_dragging = false;
                return;
            }
            auto point = parent->convertToNodeSpace(touch->getLocation());
            auto delta = point - m_startTouch;
            if (!m_dragging && (std::fabs(delta.x) > 8.f || std::fabs(delta.y) > 8.f)) m_dragging = true;
            if (m_dragging) {
                auto win = CCDirector::sharedDirector()->getWinSize();
                auto pos = m_startPosition + delta;
                pos.x = std::max(55.f, std::min(pos.x, win.width - 55.f));
                pos.y = std::max(25.f, std::min(pos.y, win.height - 25.f));
                this->setPosition(pos);
            }
        }
        void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
            if (m_pressed && !m_dragging) m_pressed->activate();
            m_pressed = nullptr;
            m_dragging = false;
        }
        void ccTouchCancelled(CCTouch* touch, CCEvent* event) override {
            m_pressed = nullptr;
            m_dragging = false;
        }
    };
}

class $modify(WhiteStructurePreviewEditorUI_v04, EditorUI) {
    struct Fields {
        DraggableMenu* menu = nullptr;
        CCMenuItemSpriteExtra* previewButton = nullptr;
        CCMenuItemSpriteExtra* confirmButton = nullptr;
        std::vector<WhiteShapeNode*> previews;
        bool previewVisible = false;
    };

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;
        auto menu = DraggableMenu::create();
        if (!menu) return true;
        menu->setID("white-preview-menu-v04"_spr);
        auto previewLabel = CCLabelBMFont::create("PREVIEW", "bigFont.fnt");
        auto confirmLabel = CCLabelBMFont::create("CONFIRM", "bigFont.fnt");
        if (!previewLabel || !confirmLabel) return true;
        previewLabel->setScale(0.35f);
        confirmLabel->setScale(0.35f);
        auto preview = CCMenuItemSpriteExtra::create(previewLabel, this, menu_selector(WhiteStructurePreviewEditorUI_v04::onPreview));
        auto confirm = CCMenuItemSpriteExtra::create(confirmLabel, this, menu_selector(WhiteStructurePreviewEditorUI_v04::onConfirm));
        if (!preview || !confirm) return true;
        auto size = CCDirector::sharedDirector()->getWinSize();
        menu->setPosition({size.width * 0.5f, size.height - 72.f});
        preview->setPosition({-48.f, 0.f});
        confirm->setPosition({48.f, 0.f});
        preview->setID("white-preview-button-v04"_spr);
        confirm->setID("white-confirm-button-v04"_spr);
        menu->addChild(preview);
        menu->addChild(confirm);
        confirm->setVisible(false);
        this->addChild(menu, 1001);
        m_fields->menu = menu;
        m_fields->previewButton = preview;
        m_fields->confirmButton = confirm;
        return true;
    }

    void update(float dt) {
        EditorUI::update(dt);
        if (!m_fields->previewVisible || !m_editorLayer) return;
        for (auto* preview : m_fields->previews) {
            if (!preview) continue;
            if (!preview->syncToLayer(this)) {
                preview->setVisible(false);
            } else {
                preview->setVisible(true);
            }
        }
    }

    void clearPreview() {
        for (auto* preview : m_fields->previews) {
            if (!preview) continue;
            if (preview->getParent()) preview->removeFromParentAndCleanup(true);
            preview->release();
        }
        m_fields->previews.clear();
        m_fields->previewVisible = false;
        if (m_fields->confirmButton) m_fields->confirmButton->setVisible(false);
    }

    std::vector<GameObject*> getSelection() {
        std::vector<GameObject*> result;
        if (m_selectedObject && m_selectedObject->getParent()) result.push_back(m_selectedObject);
        if (m_selectedObjects) {
            auto count = m_selectedObjects->count();
            for (unsigned int i = 0; i < count; ++i) {
                auto raw = m_selectedObjects->objectAtIndex(i);
                auto object = typeinfo_cast<GameObject*>(raw);
                if (object && object->getParent() &&
                    std::find(result.begin(), result.end(), object) == result.end()) {
                    result.push_back(object);
                }
            }
        }
        return result;
    }

    void onPreview(CCObject*) {
        clearPreview();
        if (!m_editorLayer) return;
        auto selected = getSelection();
        if (selected.empty()) {
            showMessage("White Structure Preview", "Select one or more blocks, ramps or spikes first.");
            return;
        }
        for (auto object : selected) {
            if (!object) continue;
            auto sourceParent = object->getParent();
            if (!sourceParent) continue;
            auto preview = WhiteShapeNode::create(object);
            if (!preview) continue;

            // Keep the custom draw node out of CCSpriteBatchNode. It is attached to
            // EditorUI and synchronized with the source object's world position every frame.
            preview->syncToLayer(this);
            preview->retain();
            this->addChild(preview, 100000);
            m_fields->previews.push_back(preview);
        }
        if (m_fields->previews.empty()) {
            clearPreview();
            showMessage("White Structure Preview", "The selected objects could not be previewed.");
            return;
        }
        m_fields->previewVisible = true;
        if (m_fields->confirmButton) m_fields->confirmButton->setVisible(true);
    }

    void onConfirm(CCObject*) {
        if (!m_fields->previewVisible || m_fields->previews.empty()) return;
        clearPreview();
    }
};
