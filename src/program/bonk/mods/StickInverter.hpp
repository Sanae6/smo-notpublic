#pragma once

#include <bonk/Mod.hpp>

namespace bm {
    struct StickInverter : public NerveMod {
        StickInverter();
        void activate() override;
        void deactivate() override;
        void sceneStart(const al::ActorInitInfo &initInfo) override;
        void exeEnable();
        void exeDisable();
    };
} // namespace bm
