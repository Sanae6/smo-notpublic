#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct GravityShift : NerveMod {
        sead::Vector3f start;
        sead::Vector3f end;
        GravityShift();
        void exePick();
        void exeLerp();
        void exeEnd();
        void deactivate() override;
        void sceneStart(const al::ActorInitInfo &initInfo) override;
    };
} // namespace bm
