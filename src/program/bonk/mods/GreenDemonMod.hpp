#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct GreenDemon;
    struct GreenDemonMod : public Mod {
        GreenDemon* oneUp;
        void sceneStart(const al::ActorInitInfo &initInfo) override;
        void activate() override;
        void marioGainsMovement() override;
        void marioBonked() override;
        void deactivate() override;
    };
} // namespace bm
