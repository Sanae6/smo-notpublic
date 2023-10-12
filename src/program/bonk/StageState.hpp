#pragma once

#include <al/Library/Scene/ISceneObj.h>
#include <al/Library/Scene/SceneObjHolder.h>
#include <lib.hpp>

namespace bm {
    enum class Mods {
        // Gameplay
        RemoveHat,
        PlayerConst,
        PoseRandomize,
        GreenDemon,
        MapPartSwap,
        CostumeChanger,
        GunMod,
        BonkFarmer, // is this even a good idea
        NoOxygen,
        Hardcore,

        // Visual
        Subscribe,
        Popups,
        ScreamingCJ, // only play this once
        TransformScreen,
        LowerTextureQuality,
        WeddingCakeMario,
        FirstPersonCamera,

        // Auditory
        DiscordSfx, // play a sound effect on a 3-minute delay
        BonkSfx,
        Twitch
    };

    class StageState : public al::ISceneObj {
        static bool isSceneWithMario(const al::ActorInitInfo& initInfo);
        const char* getSceneObjName() override { return "StageState"; }
        void initAfterPlacementSceneObj(const al::ActorInitInfo&) override;

        void update();
    };

    void stageStatePatches();

    static StageState& stageState(al::IUseSceneObjHolder* sceneObjHolder) {
        return *(StageState*)al::getSceneObj(sceneObjHolder, 0x40);
    }
} // namespace bm
