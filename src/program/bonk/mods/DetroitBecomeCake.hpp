#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct DetroitBecomeCake : public Mod {
        al::LiveActor* cakeModel;
        void activate() override;
        void sceneStart(const al::ActorInitInfo &initInfo) override;
        void control() override;
    };
} // namespace bm
