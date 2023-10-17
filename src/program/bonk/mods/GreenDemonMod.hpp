#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct GreenDemon;
    class GreenDemonMod : public Mod {
        GreenDemon* oneUp;
        void sceneStart(const al::ActorInitInfo &initInfo) override;
        void marioGainsMovement() override;
    };
} // namespace bm
