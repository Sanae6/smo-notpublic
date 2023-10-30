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
        ControllerCalcDelay::delay = this;
        ControllerCalcDelay::InstallAtSymbol("_ZN2al14NpadController9calcImpl_Ev");
    }
    void StickDelay::replace(sead::ControllerBase* c) {
        frames[currentNewFrame++] = {c->mPadHold, c->mLeftStick, c->mRightStick};
        currentNewFrame %= maxFrames;
        s32 offset = std::min((s32)par::get<u32>("StickDelayOffset", 3u), maxFrames - 1);
        s32 offsetFrame = (currentNewFrame - offset - 1) % maxFrames;
        if (offsetFrame < 0)
            offsetFrame += maxFrames;
//        Logger::log("%03d %03d\n", currentNewFrame, offsetFrame);
        Frame& currentFrame = frames[currentNewFrame];
        c->mPadHold = currentFrame.padHold;
        c->mLeftStick = currentFrame.leftStick;
        c->mRightStick = currentFrame.rightStick;
    }
} // namespace bm
