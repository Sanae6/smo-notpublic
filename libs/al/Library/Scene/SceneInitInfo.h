#pragma once

#include <al/Library/Audio/AudioDirector.h>
#include <al/Library/System/GameDataHolderBase.h>
#include <al/Library/System/GameSystemInfo.h>

#include <prim/seadSafeString.h>

namespace al {

    class ScreenCaptureExecutor;

    struct SceneInitInfo {
        const al::GameSystemInfo* mSystemInfo;
        al::GameDataHolderBase* mGameDataHolderBase;
        al::ScreenCaptureExecutor* mScreenCaptureExecutor;
        const char* mStageName;
        int mScenarioNo;
        sead::FixedSafeString<0x200> mSequenceTypeName;
        al::AudioDirector* mAudioDirector;
    };
} // namespace al