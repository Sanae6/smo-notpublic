#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct JumpyMario : NerveMod {
        JumpyMario();
        void sceneStart(const al::ActorInitInfo &initInfo) override;
        void exeEnable();
        void exeDisable();
        void deactivate() override;
    };
} // namespace bm
