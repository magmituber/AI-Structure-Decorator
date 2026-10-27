#pragma once
#include <unordered_map>
namespace autodeco {
constexpr int ID_PINCHO_NORMAL=216, ID_PINCHO_MEDIANO=217, ID_PINCHO_PEQUENO=218, ID_PINCHO_MINI=458, ID_CUADRADO=211, ID_RAMPA_NORMAL=693, ID_RAMPA_LARGA=694;
struct OverlayDef { int id; float dx=0.f; float dy=0.f; };
inline std::unordered_map<int, OverlayDef> const& overlayMap() {
    static const std::unordered_map<int, OverlayDef> map = {
        {ID_PINCHO_NORMAL,{ID_PINCHO_NORMAL}},{ID_PINCHO_MEDIANO,{ID_PINCHO_MEDIANO}},{ID_PINCHO_PEQUENO,{ID_PINCHO_PEQUENO}},{ID_PINCHO_MINI,{ID_PINCHO_MINI}},
        {ID_CUADRADO,{ID_CUADRADO}},{ID_RAMPA_NORMAL,{ID_RAMPA_NORMAL}},{ID_RAMPA_LARGA,{ID_RAMPA_LARGA}},
        {1,{ID_CUADRADO}},{8,{ID_PINCHO_NORMAL}}
    }; return map;
}
} // namespace autodeco
