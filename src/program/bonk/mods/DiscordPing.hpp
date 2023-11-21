#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct DiscordPing : public Mod, public al::IUseAudioKeeper {
        al::AudioKeeper* audioKeeper;
        al::AudioKeeper* getAudioKeeper() const override;

        void sceneStart(const al::ActorInitInfo& initInfo) override;
        void activate() override;
        void control() override;
        void pauseMe();
    };
} // namespace bm
