#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct PlayerConstMod : public Mod {
        PlayerConst basePlayerConst;
        static bool isFloat(int index);
        void sceneStart(const al::ActorInitInfo &initInfo) override;
        void activate() override;
        void randomize();
        void control() override;
    };
} // namespace bm