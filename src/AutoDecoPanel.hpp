#pragma once
#include <Geode/Geode.hpp>
#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>
#include "Config.hpp"
#include "OverlayString.hpp"
using namespace geode::prelude;
namespace autodeco {
class Panel : public CCLayer {
protected:
    static constexpr float W=124.f,H=132.f,BAR=22.f;
    EditorUI* m_editor=nullptr; std::vector<Ref<GameObject>> m_pending; bool m_dragging=false; CCLabelBMFont* m_status=nullptr;
    bool init(EditorUI* editor) {
        if(!CCLayer::init()) return false; m_editor=editor; setContentSize({W,H}); setTouchEnabled(true);
        auto bg=CCScale9Sprite::create("square02_small.png",{0.f,0.f,40.f,40.f}); bg->setContentSize({W,H}); bg->setAnchorPoint({0,0}); bg->setPosition({0,0}); bg->setOpacity(170); addChild(bg,-2);
        auto bar=CCLayerColor::create({0,0,0,110},W,BAR); bar->setPosition({0,H-BAR}); addChild(bar,-1);
        auto title=CCLabelBMFont::create("AutoDeco - mover","goldFont.fnt"); title->setScale(.4f); title->setPosition({W/2,H-BAR/2}); addChild(title);
        auto menu=CCMenu::create(); menu->setPosition({0,0}); menu->setTouchPriority(-502); addChild(menu);
        float y=H-BAR-22.f;
        auto addButton=[&](char const* label,char const* texture,SEL_MenuHandler handler){ auto spr=ButtonSprite::create(label,92,true,"bigFont.fnt",texture,24.f,.55f); auto btn=CCMenuItemSpriteExtra::create(spr,this,handler); btn->setPosition({W/2,y}); menu->addChild(btn); y-=32.f; };
        addButton("Ver blanco","GJ_button_02.png",menu_selector(Panel::onPreview)); addButton("Confirmar","GJ_button_01.png",menu_selector(Panel::onConfirm)); addButton("Cancelar","GJ_button_06.png",menu_selector(Panel::onCancel));
        m_status=CCLabelBMFont::create("","chatFont.fnt"); m_status->setScale(.5f); m_status->setPosition({W/2,9.f}); addChild(m_status);
        auto win=CCDirector::sharedDirector()->getWinSize(); float px=Mod::get()->getSavedValue<float>("panel-x",8.f), py=Mod::get()->getSavedValue<float>("panel-y",win.height*.5f); setPosition(clampPos({px,py})); return true;
    }
public:
    static Panel* create(EditorUI* editor){ auto ret=new Panel(); if(ret->init(editor)){ret->autorelease();return ret;} CC_SAFE_DELETE(ret); return nullptr; }
    void registerWithTouchDispatcher() override { CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this,-501,true); }
    bool ccTouchBegan(CCTouch* touch,CCEvent*) override { if(!isVisible()) return false; auto p=convertToNodeSpace(touch->getLocation()); if(p.x<0||p.x>W||p.y<0||p.y>H)return false; m_dragging=p.y>=H-BAR; return true; }
    void ccTouchMoved(CCTouch* touch,CCEvent*) override { if(!m_dragging)return; auto parent=getParent(); if(!parent)return; auto cur=parent->convertToNodeSpace(touch->getLocation()),prev=parent->convertToNodeSpace(touch->getPreviousLocation()); auto pos=getPosition(); setPosition(clampPos({pos.x+cur.x-prev.x,pos.y+cur.y-prev.y})); }
    void ccTouchEnded(CCTouch*,CCEvent*) override { if(m_dragging)savePosition();m_dragging=false; }
    void ccTouchCancelled(CCTouch*,CCEvent*) override { if(m_dragging)savePosition();m_dragging=false; }
protected:
    CCPoint clampPos(CCPoint p) const { auto win=CCDirector::sharedDirector()->getWinSize(); p.x=std::max(0.f,std::min(p.x,win.width-W)); p.y=std::max(0.f,std::min(p.y,win.height-H)); return p; }
    void savePosition(){auto p=getPosition();Mod::get()->setSavedValue<float>("panel-x",p.x);Mod::get()->setSavedValue<float>("panel-y",p.y);}
    void note(std::string const& msg,NotificationIcon icon=NotificationIcon::Info){Notification::create(msg,icon,1.5f)->show();}
    void setStatus(std::string const& msg){if(m_status)m_status->setString(msg.c_str());}
    bool isPending(GameObject* obj) const {for(auto const& ref:m_pending)if(ref.data()==obj)return true;return false;}
    size_t clearPending(){if(m_pending.empty())return 0;auto lel=m_editor->m_editorLayer;std::unordered_set<GameObject*> alive;if(auto all=lel->getAllObjects())for(auto* o:CCArrayExt<GameObject*>(all))alive.insert(o);size_t removed=0;for(auto& ref:m_pending){auto obj=ref.data();if(!obj||!alive.count(obj))continue;if(obj->m_isSelected)m_editor->deselectObject(obj);lel->removeObject(obj,true);++removed;}m_pending.clear();return removed;}
    void onPreview(CCObject*){
        auto lel=m_editor->m_editorLayer; std::vector<GameObject*> selected; if(auto sel=m_editor->getSelectedObjects())for(auto* o:CCArrayExt<GameObject*>(sel))selected.push_back(o); if(selected.empty()&&m_editor->m_selectedObject)selected.push_back(m_editor->m_selectedObject);
        if(selected.empty()){note("Selecciona objetos primero",NotificationIcon::Warning);return;}
        std::string all;int made=0,skipped=0;for(auto* obj:selected){if(!obj||isPending(obj))continue;auto it=overlayMap().find(obj->m_objectID);if(it==overlayMap().end()){++skipped;continue;}std::string raw=obj->getSaveString(lel).c_str();auto one=buildOverlayString(raw,it->second.id,it->second.dx,it->second.dy);if(one.empty()){++skipped;continue;}all+=one;++made;}
        if(made==0){note("Ningun objeto seleccionado es compatible",NotificationIcon::Error);setStatus("0 compatibles");return;} clearPending(); auto created=lel->createObjectsFromString(all.c_str(),true,true);if(created)for(auto* o:CCArrayExt<GameObject*>(created))m_pending.emplace_back(o);std::string msg=std::to_string(m_pending.size())+" blancos";if(skipped)msg+=" ("+std::to_string(skipped)+" omitidos)";setStatus(msg);
    }
    void onConfirm(CCObject*){if(m_pending.empty()){note("No hay vista previa que confirmar",NotificationIcon::Warning);return;}auto n=m_pending.size();m_pending.clear();setStatus("");note("Confirmados: "+std::to_string(n)+" objetos",NotificationIcon::Success);}
    void onCancel(CCObject*){if(m_pending.empty()){note("No hay vista previa que cancelar",NotificationIcon::Warning);return;}auto n=clearPending();setStatus("");note("Cancelado: "+std::to_string(n)+" objetos borrados",NotificationIcon::Info);}
};
} // namespace autodeco
