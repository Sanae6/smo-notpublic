#pragma once

#include <al/Library/LiveActor/LiveActorGroup.h>
#include <bonk/Mod.hpp>

namespace bm {
    struct PoseRandomizer : public Mod {
        al::LiveActorGroup* kit;
        void sceneStart(const al::ActorInitInfo& initInfo) override;
        void control() override;
        void hitActor(al::LiveActor* actor) const;
    };
} // namespace bm
