#pragma once

#include <al/Library/Scene/ISceneObj.h>
#include <al/Library/Scene/SceneObjHolder.h>
#include <bonk/Mod.hpp>
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
        Twitch,

        End
    };

    struct ModList {
        int modCount;
        Mod* mods[static_cast<int>(Mods::End)];

        Mod* add(Mod* mod){
            return mods[modCount++] = mod;
        }
        Mod** begin() {
            return &mods[0];
        }

        Mod** end() {
            return &mods[modCount];
        }
    };

    struct StageState : public al::ISceneObj, sead::IDisposer {
        static ModList mods;
        bool hadMario;

        StageState() : sead::IDisposer() {}
        ~StageState();
        const char* getSceneObjName() override { return "StageState"; }
        void initAfterPlacementSceneObj(const al::ActorInitInfo&) override;

        static bool isSceneWithMario(const al::ActorInitInfo& initInfo);
        void update(bool control);
        void draw(StageScene* scene, agl::DrawContext* drawContext);
        void exePlayFirstStep();
    };

    template <typename T>
    Mod* addMod() {
        return StageState::mods.add(new T());
    }
    void stageStatePatches();

    [[maybe_unused]] static StageState& stageState(al::IUseSceneObjHolder* sceneObjHolder) {
        return *(StageState*)al::getSceneObj(sceneObjHolder, 0x40);
    }
} // namespace bm
