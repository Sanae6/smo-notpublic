#pragma once

#include <al/Library/LiveActor/ActorInitInfo.h>
#include <al/Library/Nerve/NerveExecutor.h>
#include <al/Library/Player/PlayerHolder.h>
#include <bonk/ForwardDecls.hpp>
#include <game/Player/PlayerActorHakoniwa.h>
#include <game/StageScene/StageScene.h>
#include <lib.hpp>
#include <logger/Params.h>
#include <utils/Helpers.h>

namespace patch = exl::patch;
namespace inst = exl::armv8::inst;
namespace reg = exl::armv8::reg;
using namespace reg;
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
        virtual void sceneEnd() {
            sceneObjHolder = nullptr;
            marioGainedMovement = false;
        }
        void exePlayActivated() {
            if (active && !marioGainedMovement) {
                marioGainsMovement();
                marioGainedMovement = true;
            }
        }
        virtual void activate() { active = true; }
        virtual void show() { visible = true; }
        virtual void hide() { visible = false; }
        virtual void update() {
            if (pauseTimer >= 0) {
                pauseTimer--;
                return;
            }
            if (active)
                control();
        }

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

    protected:
        bool notInScene() const { return sceneObjHolder == nullptr; }
        bool inScene() const { return sceneObjHolder != nullptr; }
        virtual void renderScene() {}
        virtual void renderLayout(StageScene* scene, agl::DrawContext* drawContext) {}
        virtual void control() {}
        virtual void marioGainsMovement() {}
        void pauseForSeconds(int seconds) { pauseTimer = seconds * 60; }
        void pauseForFrames(int frames) { pauseTimer = frames; }

        PlayerActorHakoniwa* getMario() const {
            return static_cast<PlayerActorHakoniwa*>(sceneInfo.mPlayerHolder->getPlayer(0));
        }
    };

    struct NerveMod : public Mod, public al::NerveExecutor {
    protected:
        NerveMod() : al::NerveExecutor("Mod with NerveExecutor") {}
        void control() override { updateNerve(); }
    };
} // namespace bm
