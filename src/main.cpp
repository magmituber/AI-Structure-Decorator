#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/utils/web.hpp>
#include <algorithm>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace {
std::string escapeJson(std::string const& input) {
    std::string out;
    out.reserve(input.size() + 16);
    for (unsigned char c : input) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: if (c >= 0x20) out += static_cast<char>(c); break;
        }
    }
    return out;
}

void showMessage(std::string const& title, std::string const& message) {
    FLAlertLayer::create(title.c_str(), message.c_str(), "OK")->show();
}
}

class $modify(AIStructureDecoratorEditorUI, EditorUI) {
    struct Fields {
        async::TaskHolder<web::WebResponse> request;
    };

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;
        auto menu = CCMenu::create();
        if (!menu) return true;
        menu->setID("ai-decoration-menu"_spr);
        menu->setPosition({0.f, 0.f});
        auto label = CCLabelBMFont::create("AI", "goldFont.fnt");
        if (!label) return true;
        label->setScale(0.7f);
        auto button = CCMenuItemSpriteExtra::create(label, this, menu_selector(AIStructureDecoratorEditorUI::onAIDecorate));
        if (!button) return true;
        auto size = CCDirector::sharedDirector()->getWinSize();
        button->setPosition({size.width - 42.f, 68.f});
        button->setID("ai-decoration-button"_spr);
        menu->addChild(button);
        this->addChild(menu, 1000);
        return true;
    }

    void onAIDecorate(CCObject*) {
        auto apiKey = Mod::get()->getSettingValue<std::string>("gemini-api-key");
        if (apiKey.empty()) { showMessage("AI Structure Decorator", "First open the mod settings and enter your Gemini API key."); return; }
        if (!m_editorLayer || !m_selectedObjects) { showMessage("AI Structure Decorator", "Open a level in the editor and select a structure first."); return; }
        std::vector<GameObject*> selected;
        if (m_selectedObject) selected.push_back(m_selectedObject);
        for (auto object : geode::cocos::CCArrayExt<GameObject, false>(m_selectedObjects)) {
            if (object && std::find(selected.begin(), selected.end(), object) == selected.end()) selected.push_back(object);
        }
        if (selected.empty()) { showMessage("AI Structure Decorator", "Select one or more objects that form the structure you want to decorate."); return; }

        float minX = selected.front()->getPositionX(), maxX = minX;
        float minY = selected.front()->getPositionY(), maxY = minY;
        for (auto object : selected) {
            auto p = object->getPosition();
            minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
            minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
        }
        if (maxX - minX < 60.f) { minX -= 30.f; maxX += 30.f; }
        if (maxY - minY < 60.f) { minY -= 30.f; maxY += 30.f; }
        auto style = Mod::get()->getSettingValue<std::string>("decoration-prompt");
        int maxObjects = static_cast<int>(Mod::get()->getSettingValue<int64_t>("max-objects"));
        maxObjects = std::clamp(maxObjects, 1, 40);

        auto prompt = std::string("Return ONLY valid JSON with schema {\"objects\":[{\"id\":number,\"x\":number,\"y\":number}]}. Generate up to ")
            + std::to_string(maxObjects) + " decorative Geometry Dash objects. Use only object IDs from 1 to 999; never create triggers, portals, start positions, coins or gameplay objects. "
            + "x and y must be normalized numbers from 0 to 1 inside the supplied bounding box. Decorate around the structure without covering selected objects. IDs and positions only. Style: " + style
            + ". Selected object count: " + std::to_string(selected.size()) + ". Bounding box: x=" + std::to_string(minX) + ".." + std::to_string(maxX)
            + ", y=" + std::to_string(minY) + ".." + std::to_string(maxY) + ".";
        std::string body = "{\"contents\":[{\"parts\":[{\"text\":\"" + escapeJson(prompt) + "\"}]}],\"generationConfig\":{\"responseMimeType\":\"application/json\",\"temperature\":0.4}}";
        web::WebRequest request;
        request.timeout(std::chrono::seconds(45));
        request.header("Content-Type", "application/json");
        request.param("key", apiKey);
        request.bodyString(body);
        showMessage("AI Structure Decorator", "Asking Gemini to create decoration. Wait for the result.");
        m_fields->request.spawn(request.post("https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent"),
            [this, minX, maxX, minY, maxY, maxObjects](web::WebResponse response) {
                if (!response.ok()) { showMessage("AI Structure Decorator", "Gemini request failed (HTTP " + std::to_string(response.code()) + "). Check API key, internet and quota."); return; }
                auto outerResult = matjson::parse(response.string().unwrapOr(""));
                if (!outerResult) { showMessage("AI Structure Decorator", "Gemini returned an unreadable response."); return; }
                auto outer = outerResult.unwrap();
                auto generatedText = outer["candidates"][0]["content"]["parts"][0]["text"].asString().unwrapOr("");
                if (generatedText.empty()) { showMessage("AI Structure Decorator", "Gemini returned no decoration data. Try a simpler style."); return; }
                auto dataResult = matjson::parse(generatedText);
                if (!dataResult) { showMessage("AI Structure Decorator", "Gemini's result was not valid JSON. Try again."); return; }
                auto data = dataResult.unwrap();
                auto objects = data["objects"];
                if (!objects.isArray()) { showMessage("AI Structure Decorator", "The AI result did not contain an objects array."); return; }
                int created = 0;
                for (auto const& item : objects) {
                    if (created >= maxObjects) break;
                    auto idResult = item["id"].asInt();
                    auto xResult = item["x"].asDouble();
                    auto yResult = item["y"].asDouble();
                    if (!idResult || !xResult || !yResult) continue;
                    int id = static_cast<int>(idResult.unwrap());
                    double nx = xResult.unwrap(), ny = yResult.unwrap();
                    if (id < 1 || id > 999 || nx < 0.0 || nx > 1.0 || ny < 0.0 || ny > 1.0) continue;
                    CCPoint position(static_cast<float>(minX + (maxX - minX) * nx), static_cast<float>(minY + (maxY - minY) * ny));
                    if (this->m_editorLayer && this->createObject(id, position)) ++created;
                }
                if (created > 0) {
                    this->updateButtons();
                    this->updateObjectInfoLabel();
                    showMessage("AI Structure Decorator", "Created " + std::to_string(created) + " decorative objects. Check the result before saving.");
                } else { showMessage("AI Structure Decorator", "No valid decorative objects were created. Try another style or select a larger structure."); }
            });
    }
};
