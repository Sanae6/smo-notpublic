#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct JumpyMario : NerveMod {
        JumpyMario();
        void activate() override;
        void sceneStart(const al::ActorInitInfo &initInfo) override;
        void exeEnable();
        void exeDisable();
    };
} // namespace bm
