#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct MoonMode : ApplyMod {
        PlayerConst* moonPlayerConst;
        PlayerConst* original;
        void sceneStart(const al::ActorInitInfo &initInfo) override;
        void apply() override;
        void deactivate() override;
    };
} // namespace bm
