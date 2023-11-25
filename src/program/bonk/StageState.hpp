#pragma once

#include <al/Library/Scene/ISceneObj.h>
#include <al/Library/Scene/SceneObjHolder.h>
#include <bonk/Mod.hpp>
#include <lib.hpp>

namespace bm {
    struct ModList {
        int modCount;
        Mod* mods[25];

        Mod* add(Mod* mod) { return mods[modCount++] = mod; }
        Mod** begin() { return &mods[0]; }

        Mod** end() { return &mods[modCount]; }
    };

    struct StageState : public al::ISceneObj {
        static ModList mods;
        PlayerActorHakoniwa* player;
        StageScene* stageScene;

        const char* getSceneObjName() override { return "StageState"; }
        void sceneEnd(bool cleanResources);

        void initAfterPlacementSceneObj(const al::ActorInitInfo&) override;
        static bool isSceneWithMario(const al::ActorInitInfo& initInfo);
        void update(bool control);
        void draw(StageScene* scene, agl::DrawContext* drawContext);
        void exePlayFirstStep();

        bool hasMario() const { return player != nullptr; }

        static Mod* findMod(const std::type_info& typeInfo) {
            for (auto mod : mods)
                if (isSameType(mod, typeInfo))
                    return mod;
            EXL_ABORT(0x12345, "Unable to find mod!");
        }

        template <typename T>
        static T* findMod() {
            for (auto mod : mods)
                if (isSameType<T>(mod))
                    return static_cast<T*>(mod);
            EXL_ABORT(0x12345, "Unable to find mod!");
        }

        template <typename T>
        static void deactivateMod() {
            auto mod = static_cast<Mod*>(findMod<T>());
            if (mod) mod->deactivate();
        }
    };

    template <typename T>
    Mod* addMod() {
        return StageState::mods.add(alloc<T>());
    }
    void stageStatePatches();

    [[maybe_unused]] static StageState& stageState(al::IUseSceneObjHolder* sceneObjHolder) {
        return *(StageState*)al::getSceneObj(sceneObjHolder, 0x40);
    }
} // namespace bm
