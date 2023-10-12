#pragma once

#include <al/Library/Layout/LayoutKit.h>
#include <al/Library/Play/Layout/WipeHolder.h>
#include <al/Library/Screen/ScreenFunction.h>
#include <al/Library/Sequence/Sequence.h>
#include <game/GameData/GameDataHolder.h>

namespace seen {
    class AmonglaSequence : public al::Sequence {
    public:
        AmonglaSequence(const char* name);
        void init(const al::SequenceInitInfo& initInfo) override;
        void update() override;
        void drawMain() const override;

        void exeLoadScene();
        void exePlay();

        GameDataHolder* getGameDataHolder() const { return mGameDataHolder; }

        static const char* name() { return "AmonglaSequence"; }
        static AmonglaSequence* instance();

    private:
        al::ScreenCaptureExecutor* mScreenCaptureExecutor;
        GameDataHolder* mGameDataHolder;
        al::LayoutKit* mLayoutKit;
        al::WipeHolder* mWipeHolder;
    };
} // namespace seen

