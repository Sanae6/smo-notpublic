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
            al::setScale(this, sead::Vector3f::ones * par::get("CakeScale", 1.0f));
        }
    };
    struct AlwaysHidePlayer : Replace<AlwaysHidePlayer> {
        static void Callback(PlayerModelChangerHakoniwa* modelChanger) {
            modelChanger->isDirty = true;
            modelChanger->showingShadowMask = false;
            modelChanger->showingSilhouette = true;
            if (modelChanger->currentModel) al::invalidateOcclusionQuery(modelChanger->currentModel);
        }
    };
    struct HideMarioOnStart : Trampoline<HideMarioOnStart> {
        static void Callback(PlayerModelChangerHakoniwa* modelChanger, al::LiveActor* param_1, PlayerModelHolder* param_2,
                             PlayerPainPartsKeeper* param_3, PlayerCostumeInfo* param_4, IUseDimension* param_5) {
            Orig(modelChanger, param_1, param_2, param_3, param_4, param_5);
            modelChanger->isDirty = true;
            modelChanger->showingShadowMask = false;
            modelChanger->showingModel = false;
            if (modelChanger->currentModel) al::invalidateOcclusionQuery(modelChanger->currentModel);
        }
    };
    void DetroitBecomeCake::activate() {
        Mod::activate();
        updateAlways = true;
        patch::CodePatcher patcher(0x45dcd0); // disable model on load
        patcher.Seek(0x45e494);
        patcher.WriteInst(inst::Ret());
        AlwaysHidePlayer::InstallAtSymbol("_ZN26PlayerModelChangerHakoniwa9showModelEv");
        HideMarioOnStart::InstallAtSymbol("_ZN26PlayerModelChangerHakoniwaC1EPKN2al9LiveActorEP17PlayerModelHolderP21Pl"
                                          "ayerPainPartsKeeperP17PlayerCostumeInfoPK13IUseDimension");

        if (inScene()) {
            cakeModel->appear();
            getMario()->mModelChanger->isDirty = true;
            getMario()->mModelChanger->showingModel = false;
            getMario()->mModelChanger->showingShadowMask = false;
            getMario()->mModelChanger->syncHost(true);
            al::invalidateOcclusionQuery(getMario()->mModelChanger->currentModel);
        }
    }
    void DetroitBecomeCake::sceneStart(const al::ActorInitInfo& initInfo) {
        Mod::sceneStart(initInfo);
        cakeModel = new CakeMario();

        cakeModel->init(initInfo);

        if (active)
            cakeModel->appear();
    }
} // namespace bm
