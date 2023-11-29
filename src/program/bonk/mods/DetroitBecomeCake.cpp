#include <al/Library/Demo/DemoFunction.h>
#include <al/Library/LiveActor/ActorModelFunction.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <bonk/mods/DetroitBecomeCake.hpp>
#include <logger/Logger.hpp>

namespace bm {
    struct CakeMario : public al::LiveActor {
        const char* model;
        CakeMario(const char* model) : al::LiveActor("ケーキ"), model(model) {}
        PlayerActorHakoniwa* getMario() const {
            return static_cast<PlayerActorHakoniwa*>(mSceneInfo->mPlayerHolder->getPlayer(0));
        }
        void init(const al::ActorInitInfo& info) override {
            al::initActorWithArchiveName(this, info, model, nullptr);

            al::copyPose(this, getMario());
            al::setScale(this, sead::Vector3f::ones * par::get("CakeScale", 1.0f));
            al::registActorToDemoInfo(this, info);
            kill();
        }
        void control() override {
            al::LiveActor::control();
            al::copyPose(this, getMario());
            *al::getScalePtr(this) = al::getScale(getMario()) * par::get("CakeScale", 1.0f);
        }
    };
    void DetroitBecomeCake::activate() {
        Mod::activate();

        if (inScene())
            getActiveModel()->appear();
    }
    void DetroitBecomeCake::sceneStart(const al::ActorInitInfo& initInfo) {
        Mod::sceneStart(initInfo);
        cakeModel = alloc<CakeMario>("WeddingCake");
        globeModel = alloc<CakeMario>("ShineTowerGlobe");

        cakeModel->init(initInfo);
        globeModel->init(initInfo);

        if (active)
            getActiveModel()->appear();
    }
    void DetroitBecomeCake::deactivate() {
        Mod::deactivate();
        if (inScene())
            getActiveModel()->kill();
    }
} // namespace bm
