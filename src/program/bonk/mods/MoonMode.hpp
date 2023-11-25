#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct MoonMode : ApplyMod {
        PlayerConst* moonPlayerConst;
        void sceneStart(const al::ActorInitInfo &initInfo) override;
        void apply() override;
    };
} // namespace bm
