#include <al/Library/Light/ActorPrepassLightKeeper.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/Math/MathUtil.h>
#include <al/Library/Math/MathVectorUtil.h>
#include <al/Library/Player/PlayerHolder.h>
#include <al/Library/Resource/ResourceHolder.h>
#include <gfx/seadCamera.h>
#include <heap/seadHeapMgr.h>
#include <logger/Params.h>
#include <spook/SpookState.hpp>
#include <utils/ForwardDecls.hpp>
#include <utils/Helpers.h>

namespace sp {
    sead::LookAtCamera* camera = nullptr;
    void SpookState::initAfterPlacementSceneObj(const al::ActorInitInfo& initInfo) {
        if (!isSameType<StageScene>(scene))
            return;
        player = static_cast<PlayerActorHakoniwa*>(al::getPlayerActor(initInfo.mActorSceneInfo.mPlayerHolder, 0));

        auto& lights = player->mActorPrePassLightKeeper->mLights;
        flashlight = reinterpret_cast<al::LppSpot*>(lights[0]);
        postProcessingFilter = unsafeRef<al::PostProcessingFilter*>(getGraphicsSystemInfo(initInfo), 0x2d0);
        Logger::log("Light count: %d/%d\n", lights.size(), lights.capacity());
        for (int i = 0; i < lights.size(); i++) {
            Logger::log("Mario's Light: %s %s %s\n", typeid(*lights[i]).name(), lights[i]->name->value,
                        BTOC(lights[i]->killedByUser), lights[i]);
        }

        firstPerson = alloc<FirstPersonState>(this, initInfo);
        camera = nullptr;

        follower = alloc<al::LiveActor>("LightFollower");
        al::initActorWithArchiveName(follower, initInfo, "TankReviveCtrl", nullptr);
        follower->appear();
    }
    void SpookState::update() {
        firstPerson->playerMovement();
        al::validatePostProcessingFilter(scene);
        postProcessingFilter->currentPreset = par::get("Filter", 0);
        if (postProcessingFilter->currentPreset > 17)
            postProcessingFilter->currentPreset = 17;
        if (!par::get("UpdateFlashlight", false))
            return;

        camera = al::getLookAtCamera(player, 0);

        flashlight->currentColor = flashlight->targetColor = flashlight->color->value =
            sead::Color4f(par::get("ColorR", 10000.0f), par::get("ColorG", 8000.0f), par::get("ColorB", 9000.0f), 1.0f);
        flashlight->param.degree->value = par::get("Degree", 50.0f);
        flashlight->param.length->value = par::get("Length", 1000.0f);
        flashlight->randomCeil->value = par::get("RandomCeil", 0.0f);
        flashlight->param.angleDamp->value = par::get("AngleDamp", 1.0f);
        flashlight->param.specularExpansion->value = par::get("SpecExpansion", 0.0f);
        flashlight->param.pcf->value = par::get("Pcf", 1.5f);
        auto* rotateOffset = &flashlight->rotateOffset->value;
        rotateOffset->x = par::get("RotateX", 0.0f);
        rotateOffset->y = par::get("RotateY", 0.0f);
        rotateOffset->z = par::get("RotateZ", 0.0f);
        if (par::get("UseActorDirection", false)) {
            if (flashlight->mtxConnector->targetMtx != player->getBaseMtx())
                flashlight->mtxConnector->init(player->getBaseMtx(), sead::Matrix34f::ident);
        } else {
//            sead::Quatf quat;
            sead::Vector3f rot, up;
            al::calcCameraLookDir(&rot, player, 0);
//            al::calcUpDir(&up, player);
//            al::setTrans(follower, al::getTrans(player));
//            al::makeQuatFrontUp(&quat, rot, up);
//            al::setQuat(follower, quat);
//            al::copyPose(follower, player);
            noRotateMtx.makeSRT(sead::Vector3f::ones, rot, al::getTrans(player));
            follower->calcAnim();
            if (flashlight->mtxConnector->targetMtx != &noRotateMtx)
                flashlight->mtxConnector->init(&noRotateMtx, sead::Matrix34f::ident);
//            if (par::get("LogDirection", false))
//                Logger::log("playerPos: %.02f %.02f %.02f\n", rot.x, rot.y, rot.z);
        }
        if (par::get("LogGarbage", false)) {
            //            auto facing = camera->getPos() - camera->getAt();
            //            facing.normalize();
            //            auto& mtx = camera->mMatrix.a;
            //            auto lookDir = sead::Vector3f(-mtx[10], -mtx[11], -mtx[12]);
            auto playerPos = al::getTrans(player);
            Logger::log("playerPos: %.02f %.02f %.02f\n", playerPos.x, playerPos.y, playerPos.z);
            Logger::log("rotateOffset: %.02f %.02f %.02f\n", rotateOffset->x, rotateOffset->y, rotateOffset->z);
            //            Logger::log("camera: %.02f %.02f %.02f %.02f %.02f %.02f\n", camera->mPos.x, camera->mPos.y,
            //            camera->mPos.z,camera->mAt.x, camera->mAt.y, camera->mAt.z); Logger::log("Normalized facing:
            //            %.02f %.02f %.02f\n", facing.x, facing.y, facing.z); auto* light =
            //            player->mActorPrePassLightKeeper->mLights[0]; Logger::log("we hit that ass %s %s %s %.02f
            //            %.02f %.02f %.02f\n",
            //                        typeid(*light->getNerveKeeper()->getCurrentNerve()).name(),
            //                        BTOC(player->mActorPrePassLightKeeper->mIsIgnoreHideModel->value),
            //                        BTOC(player->mActorPrePassLightKeeper->mIsIgnorePrePassLightYaml),
            //                        light->currentColor.r, light->currentColor.g, light->currentColor.b,
            //                        light->currentColor.a);
        }
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

            spookyState(scene).update();
        }
    };

    void requestConnectOffsetModifier(al::MtxConnector* connector, sead::Vector3f* trans, sead::Quatf* quat,
                                      sead::Vector3f* scale, const sead::Vector3f& transOffset,
                                      const sead::Vector3f& rotateOffset) {

        //        if (!camera || par::get("UseActorDirection", true)) {
        connector->calcConnectInfo(trans, quat, scale, transOffset, rotateOffset);
        //            return;
        //        }

        //        connector->calcConnectInfo(trans, quat, scale, transOffset, roleyP);
    }

    void spookyInit() {
        StageStateCreate::InstallAtSymbol("_ZN2al5Scene18initSceneObjHolderEPNS_14SceneObjHolderE");
        StageScenePlay::InstallAtSymbol("_ZN10StageScene7controlEv");

        patch::CodePatcher patcher(0xa93450);
        patcher.Write(inst::Movz(W0, 1));
        patcher.Seek(0xa582c8);
        patcher.Write(inst::Movz(W8, 1));

        //        patcher.Seek("_ZN11LppFunction10requestLppIN2al12LppSpotParamEEEvPT_PNS1_16PrePassLightBaseE", 0x60);
        //        patcher.BranchLinkInst((void*)&requestConnectOffsetModifier);

        patcher.Seek(0x97bd4c);
        patcher.Write(0x52B85648); // -30 -> -89 for min angle
        patcher.SeekRel(16);
        patcher.Write(0x52A85648); // 75 -> 89 for max angle
        patcher.SeekRel(4);
        patcher.Write(0x52A86188); // 180 -> 110 for height

        patcher.Seek("_ZNK11PlayerInput25isTriggerCameraSubjectiveEv", 0x0);
        patcher.WriteInst(inst::MovRegister(reg::X0, reg::None64));
        patcher.WriteInst(inst::Ret());

        patcher.Seek(0x97c0a4); // disable * 50f for CameraPoserSubjective offset
        patcher.WriteInst(inst::Nop());
        patcher.WriteInst(inst::Nop());
        patcher.WriteInst(inst::Nop());
        patcher.WriteInst(inst::Nop());
        patcher.Seek("_ZN20PlayerActionFunction13isOppositeDirERKN4sead7Vector3IfEES4_", 0x0);
        patcher.WriteInst(inst::MovRegister(reg::X0, reg::None64));
        patcher.WriteInst(inst::Ret());
    }
} // namespace sp
