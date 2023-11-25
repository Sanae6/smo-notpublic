#pragma once

#include <bonk/Mod.hpp>
#include <controller/seadControllerBase.h>

namespace bm {
    struct StickDelay : public Mod {
        static const s32 maxFrames = 20;
        struct Frame {
            sead::BitFlag32 padHold = 0;
            sead::Vector2f leftStick = sead::Vector2f::zero;
            sead::Vector2f rightStick = sead::Vector2f::zero;
        };
        Frame frames[maxFrames] {};

        void activate() override;
        void replace(sead::ControllerBase* c);

        friend struct ControllerCalcDelay;
    };
} // namespace bm
