#include <bonk/mods/MoonMode.hpp>
#include <game/Player/PlayerFunction.h>

namespace bm {
    void MoonMode::sceneStart(const al::ActorInitInfo& initInfo) {
        moonPlayerConst = PlayerFunction::createMarioConst("Moon");
        ApplyMod::sceneStart(initInfo);
        original = *getMario()->mPlayerConst;
    }
    void MoonMode::apply() { *getMario()->mPlayerConst = *moonPlayerConst; }
    void MoonMode::deactivate() {
        Mod::deactivate();
        if (inScene())
            *getMario()->mPlayerConst = original;
    }
} // namespace bm
