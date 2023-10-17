#pragma once

#include <bonk/Mod.hpp>

namespace bm {
    class GunMod : public Mod {
        void activate() override;
void sceneStart(const al::ActorInitInfo &initInfo) override;
    };
} // namespace bm
