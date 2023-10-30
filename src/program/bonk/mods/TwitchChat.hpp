#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct TwitchChat : public Mod {
        al::SimpleLayoutText* layout;
        TwitchChat();
        void* loadFont();
        void sceneStart(const al::ActorInitInfo &initInfo) override;
        void update() override;
    };
} // namespace bm
