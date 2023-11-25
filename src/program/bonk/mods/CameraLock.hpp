#pragma once

#include <bonk/Mod.hpp>
#include <al/Library/Camera/CameraTargetBase.h>

namespace bm {
    struct DummyCameraTarget : public al::CameraTargetBase {
        sead::Vector3f lastTrans;
        const char * getTargetName() override { return "鬼ヶ島"; }
        void calcTrans(sead::Vector3f *trans) override {
            *trans = lastTrans;
        }
    };
    struct CameraLock : public NerveMod {
    public:
        DummyCameraTarget target;

        void activate() override;
        void sceneStart(const al::ActorInitInfo &initInfo) override;

        void exeLock();
        void exeUnlocked();
    };
} // namespace bm
