#include <al/Library/Controller/JoyPadUtil.h>
#include <bonk/mods/StickDelay.hpp>
#include <controller/seadControllerMgr.h>
#include <logger/Logger.hpp>

namespace bm {
    struct ControllerCalcDelay : public Trampoline<ControllerCalcDelay> {
        static StickDelay* delay;
        static void Callback(sead::ControllerBase* c) {
            Orig(c);
            if (c == sead::ControllerMgr::instance()->getController(al::getMainControllerPort()))
                delay->replace(c);
        }
    };
    StickDelay* ControllerCalcDelay::delay = nullptr;
    void StickDelay::activate() {
        Mod::activate();
        ControllerCalcDelay::delay = this;
        ControllerCalcDelay::InstallAtSymbol("_ZN2al14NpadController9calcImpl_Ev");
    }
    void StickDelay::replace(sead::ControllerBase* c) {
        if (!active) return;
        std::swap(c->mPadHold, frames[0].padHold);
        std::swap(c->mLeftStick, frames[0].leftStick);
        std::swap(c->mRightStick, frames[0].rightStick);
        s32 offsetMax = std::min((s32)par::get<u32>("StickDelayOffset", 3u), maxFrames - 1);
        c->mPadHold = frames[offsetMax - 1].padHold;
        c->mLeftStick = frames[offsetMax - 1].leftStick;
        c->mRightStick = frames[offsetMax - 1].rightStick;
        for (int i = offsetMax - 1; i > 0; i--) {
            std::swap(frames[i - 1], frames[i]);
        }
    }
} // namespace bm
