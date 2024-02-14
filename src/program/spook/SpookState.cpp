#include "al/Library/Nerve/NerveSetupUtil.h"
#include "math/seadVector.h"
#include "spook/SePlayObjSpooky.hpp"
#include <al/Library/Controller/JoyPadUtil.h>
#include <al/Library/Light/ActorPrepassLightKeeper.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/Math/MathUtil.h>
#include <al/Library/Math/MathVectorUtil.h>
#include <al/Library/Player/PlayerHolder.h>
#include <al/Library/Resource/ResourceHolder.h>
#include <game/Player/CapFunction.h>
#include <gfx/seadCamera.h>
#include <heap/seadHeapMgr.h>
#include <logger/Params.h>
#include <spook/SpookState.hpp>
#include <utils/ForwardDecls.hpp>
#include <utils/Helpers.h>
#include <al/Library/Audio/AudioDirector.h>

namespace sp {
    sead::LookAtCamera* camera = nullptr;
    void SpookState::initAfterPlacementSceneObj(const al::ActorInitInfo& initInfo) {
        if (!isSameType<StageScene>(scene))
            return;
        player = static_cast<PlayerActorHakoniwa*>(al::getPlayerActor(initInfo.mActorSceneInfo.mPlayerHolder, 0));
        auto& lights = player->mActorPrePassLightKeeper->mLights;
        flashlight = reinterpret_cast<al::LppSpot*>(lights[0]);
        flashlight->requestKillByUser();
        postProcessingFilter = unsafeRef<al::PostProcessingFilter*>(getGraphicsSystemInfo(initInfo), 0x2d0);
        Logger::log("Light count: %d/%d\n", lights.size(), lights.capacity());
        for (int i = 0; i < lights.size(); i++) {
            Logger::log("Mario's Light: %s %s %s\n", typeid(*lights[i]).name(), lights[i]->name->value,
                        BTOC(lights[i]->killedByUser), lights[i]);
        }

        firstPerson = alloc<FirstPersonState>(this, initInfo);
        camera = nullptr;
        startSpook();
        audioDirector = initInfo.mAudioDirector;
        spookyPlay = alloc<SePlayObjSpooky>(player);
    }
    void SpookState::update() {
        if (!isNerve<StageSceneNrvPlay>(scene)) return;
        firstPerson->playerMovement();
        al::validatePostProcessingFilter(scene);
        postProcessingFilter->currentPreset = par::get("Filter", 0);
        if (postProcessingFilter->currentPreset > 17)
            postProcessingFilter->currentPreset = 17;

        player->mPlayerConst->mNormalMaxSpeed = par::get("MaxSpeed", 14.0f);
        if (!par::get("UpdateFlashlight", false) || !isSpookActive)
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
        flashlight->mtxConnector->init(player->getBaseMtx(), sead::Matrix34f::ident);

        if (al::isPadTriggerA(-1) || al::isPadTriggerB(-1)) {
            if (flashlight->killedByUser) flashlight->requestAppearByUser(0);
            else flashlight->requestKillByUser(0);
            Logger::log("Playing sound %s\n", par::get("SeName", "LunaticPrincess"));
            alSeFunction::startSeFromUpperLayerSeKeeper(player, par::get("SeName", "LunaticPrincess"));
        }

        if (par::clicked("PlaySound")) {
            // sead::Vector3f place = sead::Vector3f(al::getRandom(-100.0f, 100.0f), 0, al::getRandom(-100.0f, 100.0f));
            // audioDirector->mSeDirector->topLevelSeKeeper->requestPlaySe(par::get("SeName", "LunaticPrincess"), 0, 0, 0, 0, false, 0, nullptr);
            al::startSe(player, par::get("SeName", "LunaticPrincess"));
            alSeFunction::startSeFromUpperLayerSeKeeper(player, par::get("SeName", "LunaticPrincess"));
        }
    }
    void SpookState::startSpook() {
        isSpookActive = true;
        patch::CodePatcher patcher(0xa93450);
        patcher.Seek("_ZNK11PlayerInput25isTriggerCameraSubjectiveEv", 0x0);
        patcher.WriteInst(inst::MovRegister(reg::X0, reg::None64));
        patcher.WriteInst(inst::Ret());
        patcher.Seek("_ZN20PlayerActionFunction13isOppositeDirERKN4sead7Vector3IfEES4_", 0x0);
        patcher.WriteInst(inst::MovRegister(reg::X0, reg::None64));
        patcher.WriteInst(inst::Ret());
        patcher.Seek("_ZNK11PlayerInput13isTriggerJumpEv", 0x0);
        patcher.WriteInst(inst::MovRegister(reg::X0, reg::None64));
        patcher.WriteInst(inst::Ret());
        patcher.Seek("_ZNK11PlayerInput16isTriggerRollingEb", 0x14);
        patcher.WriteInst(inst::Movz(reg::W8, 1));
        scene->mGameDataHolder.mData->mDataFile->mIsEnableCap = false;
        CapFunction::putOnCapPlayer(player->mHackCap, player->mPlayerAnimator);
    }
    void SpookState::stopSpook() {
        isSpookActive = false;
        patch::CodePatcher patcher(0xa93450);
        patcher.Seek("_ZNK11PlayerInput25isTriggerCameraSubjectiveEv", 0x0);
        patcher.Write(0xF81E0FF3);
        patcher.Write(0xA9017BFD);
        patcher.Seek("_ZN20PlayerActionFunction13isOppositeDirERKN4sead7Vector3IfEES4_", 0x0);
        patcher.Write(0x2D400420);
        patcher.Write(0xD000A348);
        patcher.Seek("_ZNK11PlayerInput13isTriggerJumpEv", 0x0);
        patcher.Write(0xA9BE4FF4);
        patcher.Write(0xA9017BFD);
        patcher.Seek("_ZNK11PlayerInput16isTriggerRollingEb", 0x14);
        patcher.Write(0x39426268);
        scene->mGameDataHolder.mData->mDataFile->mIsEnableCap = true;
        player->mHackCap->hide(false);
        player->mPlayerAnimator->forceCapOn();
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

    struct CheckAddonLoadStates : public Trampoline<CheckAddonLoadStates> {
        static bool Callback(s32 type, const char* name, al::SeadAudioPlayer* player) {
            bool res = Orig(type, name, player);
            Logger::log("%s: %s\n", name, BTOC(res));
            return res;
        }
    };

    void spookyInit() {
        /*
         * I think at this point all I'd really need a more cosmetic changes to help with the vibe of the mod.
         * Would you be willing to lower the players move speed, make it so that they can no longer jump + throw cappy,
         * and then also make it so that the jump button turns on and off the flashlight?
         * I'm also slightly interested in the idea of me being able to use the GUI to play sounds in game so that I can
         * mess with them while they go through the level. I don't have any sounds picked out yet,
         * but it seems like a fun system to implement for the type of level it is.
         */
        StageStateCreate::InstallAtSymbol("_ZN2al5Scene18initSceneObjHolderEPNS_14SceneObjHolderE");
        StageScenePlay::InstallAtSymbol("_ZN10StageScene7controlEv");
        CheckAddonLoadStates::InstallAtOffset(0x808cc0);

        patch::CodePatcher patcher(0xa93450);
        patcher.Write(inst::Movz(W0, 1));
        patcher.Seek(0xa582c8);
        patcher.Write(inst::Movz(W8, 1));

        patcher.Seek(0x97bd4c);
        patcher.Write(0x52B85648); // -30 -> -89 for min angle
        patcher.SeekRel(16);
        patcher.Write(0x52A85648); // 75 -> 89 for max angle
        patcher.SeekRel(4);
        patcher.Write(0x52A86188); // 180 -> 110 for height

        patcher.Seek(0x97c0a4); // disable * 50f for CameraPoserSubjective offset
        patcher.WriteInst(inst::Nop());
        patcher.WriteInst(inst::Nop());
        patcher.WriteInst(inst::Nop());
        patcher.WriteInst(inst::Nop());
    }
} // namespace sp
