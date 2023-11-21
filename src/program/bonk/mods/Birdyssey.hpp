#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct Birdyssey : Mod {
        al::LiveActor** birds = nullptr;
        al::LiveActor* findDeadBird();
        void sceneStart(const al::ActorInitInfo &initInfo) override;
        void control() override;
    };
} // namespace bm
