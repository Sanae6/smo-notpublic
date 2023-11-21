#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct RemoveHat : public NerveMod {

        bool levelStartedWithHat = false;
        RemoveHat();
        void sceneStart(const al::ActorInitInfo &initInfo) override;
        void sceneEnd(bool b) override;
        void activate() override;
        void update() override;
        void control() override;
        void exeHide();
        void exeShow();
    };
} // namespace bm