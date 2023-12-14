#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/Player/PlayerHolder.h>
#include <heap/seadHeapMgr.h>
#include <logger/Params.h>
#include <spook/SpookState.hpp>
#include <utils/ForwardDecls.hpp>
#include <utils/Helpers.h>

namespace sp {
    void SpookState::initAfterPlacementSceneObj(const al::ActorInitInfo& initInfo) {
        if (!isSameType<StageScene>(scene))
            return;
        player = static_cast<PlayerActorHakoniwa*>(al::getPlayerActor(initInfo.mActorSceneInfo.mPlayerHolder, 0));
        flashlight = static_cast<al::PrePassSpotLight*>(al::createActorFunction<al::PrePassSpotLight>("Flashlight"));
        al::initCreateActorNoPlacementInfoNoViewId(flashlight, initInfo);
        flashlight->appear();
        flashlight->prePassLight->requestAppearByUser();
    }
    void SpookState::update() {
        if (par::get("LogGarbage", false)) Logger::log("we hit that ass %d\n", flashlight->getFlags()->isDead);
        auto light = flashlight->prePassLight;
        auto& param = flashlight->prePassLight->param;
        param.length->ref() = par::get("FlLength", 10000);
        param.degree->ref() = par::get("FlDegree", 30);
        light->color->ref() =
            sead::Color4f(par::get("FlRed", 13000), par::get("FlGreen", 13000), par::get("FlBlue", 13000), 1.0f);
        al::setTrans(flashlight, al::getTrans(player) + sead::Vector3f(0, 300, 0));

    }

    struct StageStateCreate : public Trampoline<StageStateCreate> {
        static al::SceneObjHolder* Callback(al::Scene* scene, al::SceneObjHolder* holder) {
            Orig(scene, holder);

            sead::ScopedCurrentHeapSetter setter(al::getSceneHeap());
            auto state = alloc<SpookState>();
            if (isSameType<StageScene>(scene))
                state->scene = static_cast<StageScene*>(scene);
            holder->setSceneObj(state, 0x40);
            return holder;
        }
    };

    struct StageScenePlay : public Trampoline<StageScenePlay> {
        static void Callback(StageScene* scene) {
            Orig(scene);
            if (spookyState(scene).flashlight != nullptr) {
                spookyState(scene).update();
            }
        }
    };

    void spookyInit() {
        StageStateCreate::InstallAtSymbol("_ZN2al5Scene18initSceneObjHolderEPNS_14SceneObjHolderE");
        StageScenePlay::InstallAtSymbol("_ZN10StageScene7exePlayEv");

        patch::CodePatcher patcher(0xa93450);
        patcher.Write(inst::Movz(W0, 1));
        patcher.Seek(0xa582c8);
        patcher.Write(inst::Movz(W0, 1));
    }
} // namespace sp
