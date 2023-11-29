#pragma once

#include <al/Library/LiveActor/ActorInitInfo.h>
#include <al/Library/Nerve/NerveExecutor.h>
#include <al/Library/Player/PlayerHolder.h>
#include <basis/seadNew.h>
#include <bonk/ForwardDecls.hpp>
#include <bonk/ModSaveData.hpp>
#include <game/Player/PlayerActorHakoniwa.h>
#include <game/StageScene/StageScene.h>
#include <lib.hpp>
#include <logger/Params.h>
#include <utils/Helpers.h>

template <typename T>
using Trampoline = exl::hook::impl::TrampolineHook<T>;
template <typename T>
using Replace = exl::hook::impl::ReplaceHook<T>;

namespace bm {
    struct Mod : public al::IUseSceneObjHolder {
    protected:
        bool active = false;
        bool visible = true;
        bool updateAlways = false;
        bool marioGainedMovement;
        al::SceneObjHolder* sceneObjHolder;
        al::ActorSceneInfo sceneInfo;
        int pauseTimer = 0;

    public:
        bool shouldUpdateOnControl() const { return updateAlways; }
        al::SceneObjHolder* getSceneObjHolder() const override { return sceneObjHolder; }
        virtual void sceneStart(const al::ActorInitInfo& initInfo) {
            sceneObjHolder = initInfo.mActorSceneInfo.mSceneObjHolder;
            sceneInfo = initInfo.mActorSceneInfo;
        }
        virtual void sceneEnd(bool cleanResources) {
            sceneObjHolder = nullptr;
            marioGainedMovement = false;
        }
        void exePlayActivated() {
            if (active && !marioGainedMovement) {
                marioGainsMovement();
                marioGainedMovement = true;
            }
        }
        virtual void activate() {
            active = true;
            ModSaveData::instance().setModActive(this, true);
        }
        virtual void deactivate() {
            active = false;
            ModSaveData::instance().setModActive(this, false);
        }
        virtual void show() { visible = true; }
        virtual void hide() { visible = false; }
        virtual void update() {
            if (isPaused()) {
                pauseTimer--;
                return;
            }
            if (active)
                control();
        }

        virtual void marioBonked() {}

        void renderToScene() {
            if (active && visible) {
                renderScene();
            }
        }

        void renderToScreen(StageScene* stageScene, agl::DrawContext* drawContext) {
            if (active && visible) {
                renderLayout(stageScene, drawContext);
            }
        }

        bool notInScene() const { return sceneObjHolder == nullptr; }
        bool inScene() const { return sceneObjHolder != nullptr; }
        bool isActive() const { return active; }

        int modId = 0;

    protected:
        virtual void renderScene() {}
        virtual void renderLayout(StageScene* scene, agl::DrawContext* drawContext) {}
        virtual void control() {}
        virtual void marioGainsMovement() {}
        bool isPaused() const { return pauseTimer >= 0; }
        void pauseForSeconds(int seconds) { pauseTimer = seconds * 60; }
        void pauseForFrames(int frames) { pauseTimer = frames; }
        void unpause() { pauseTimer = 0; }

        PlayerActorHakoniwa* getMario() const {
            return static_cast<PlayerActorHakoniwa*>(sceneInfo.mPlayerHolder->getPlayer(0));
        }

        GameDataHolder* getGameDataHolder() const { return static_cast<GameDataHolder*>(sceneInfo.mGameDataHolder); }
    };

    struct ApplyMod : public Mod {
        void activate() {
            Mod::activate();
            if (inScene())
                apply();
        }
        void sceneStart(const al::ActorInitInfo& initInfo) {
            Mod::sceneStart(initInfo);
            if (active)
                apply();
        }
        virtual void apply() = 0;
    };

    struct NerveMod : public Mod, public al::NerveExecutor {
    protected:
        NerveMod() : al::NerveExecutor("Mod with NerveExecutor") {}
        void control() override { updateNerve(); }
    };
} // namespace bm
