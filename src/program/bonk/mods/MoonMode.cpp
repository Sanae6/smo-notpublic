#include <bonk/mods/MoonMode.hpp>
#include <game/Player/PlayerFunction.h>

namespace bm {
    void MoonMode::sceneStart(const al::ActorInitInfo& initInfo) {
        moonPlayerConst = PlayerFunction::createMarioConst("Moon");
        original = alloc<PlayerConst>();
        *original = *getMario()->mPlayerConst;
        ApplyMod::sceneStart(initInfo);
    }
    void MoonMode::apply() { *getMario()->mPlayerConst = *moonPlayerConst; }
    void MoonMode::deactivate() { Mod::deactivate(); if (inScene()) *getMario()->mPlayerConst = *original; }
} // namespace bm
