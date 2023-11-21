#include <al/Library/Demo/DemoFunction.h>
#include <al/Library/LiveActor/ActorModelFunction.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <bonk/mods/DetroitBecomeCake.hpp>
#include <logger/Logger.hpp>

namespace bm {
    struct CakeMario : public al::LiveActor {
        CakeMario() : al::LiveActor("ケーキ") {}
        PlayerActorHakoniwa* getMario() const {
            return static_cast<PlayerActorHakoniwa*>(mSceneInfo->mPlayerHolder->getPlayer(0));
        }
        void init(const al::ActorInitInfo &info) override {
            al::initActorWithArchiveName(this, info, "WeddingCake", nullptr);

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
            cakeModel->appear();
    }
    void DetroitBecomeCake::sceneStart(const al::ActorInitInfo& initInfo) {
        Mod::sceneStart(initInfo);
        cakeModel = new CakeMario();

        cakeModel->init(initInfo);

        if (active)
            cakeModel->appear();
    }
} // namespace bm
