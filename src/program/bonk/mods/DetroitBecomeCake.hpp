#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct DetroitBecomeCake : public Mod {
        al::LiveActor* cakeModel;
        al::LiveActor* globeModel;
        bool isGlobe = false;
        void activate() override;
        void sceneStart(const al::ActorInitInfo &initInfo) override;
        al::LiveActor* getActiveModel() const { return isGlobe ? globeModel : cakeModel; }
        void deactivate() override;
    };
} // namespace bm
