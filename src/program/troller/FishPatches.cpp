#include "ForwardDecls.h"
#include <al/Library/Controller/JoyPadUtil.h>
#include <al/Library/LiveActor/ActorMovementFunction.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/LiveActor/ActorSensorFunction.h>
#include <al/Library/LiveActor/LiveActor.h>
#include <game/Player/CapFunction.h>
#include <game/Player/PlayerActorHakoniwa.h>
#include <game/Player/PlayerDamageKeeper.h>
#include <game/Player/PlayerFunction.h>
#include <game/Player/States/PlayerStateHack.h>
#include <game/StageScene/StageScene.h>
#include <gfx/seadTextWriter.h>
#include <lib.hpp>
#include <logger/Logger.hpp>
#include <logger/Params.h>
#include <program/troller/Helpers.h>
#include <program/troller/StageState.h>
#include <rs/util/SensorUtil.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"
namespace patch = exl::patch;
namespace inst = exl::armv8::inst;
namespace reg = exl::armv8::reg;

#pragma region Fish Nerves
MAKE_NERVE(Pukupuku, CaptureStart);
MAKE_NERVE(Pukupuku, CaptureWait);
namespace {
    MAKE_NERVE_FAKE(Pukupuku, Swoon);
    MAKE_NERVE_FAKE(Pukupuku, Revive);
} // namespace
#pragma endregion

#pragma region Fish Management

struct PlayerControlEnableFishy : exl::hook::impl::TrampolineHook<PlayerControlEnableFishy> {
    static bool shouldEnableFishy(al::LiveActor* actor) {
        return StageState::getFishEnableAreaGroup(actor) ? StageState::isInFishEnableArea(actor)
                                                         : al::isPadTriggerLeft(-1);
    }

    static void killFishy(StageState& state) {
        al::setNerve(state.fish, &PukupukuNrvDoNothing::sInstance);
        if (al::isExistPrePassLight(state.fish, "Front") && al::isActivePrePassLight(state.fish, "Front")) {
            al::killPrePassLight(state.fish, "Front", -1);
            al::tryGetSubActor(state.fish, "ライト")->kill();
        }
        state.fish->kill();
    }

    static void Callback(PlayerActorHakoniwa* player) {
        Orig(player);

        auto& state = stageState(player);
        if (!state.hasFish()) {
            Logger::log("Has mario but no fish...?\n");
            return;
        }

        if (isNerve<PlayerActorHakoniwaNrvAbyss>(player) || player->mPlayerDamageKeeper->mHitPoint == 0)
            state.hasDied = true;

        if (shouldEnableFishy(player) && !state.isFishCapture() && !rs::isActiveDemo(player) &&
            !isNerve<PlayerActorHakoniwaNrvHack>(player) && !rs::isPlayer2D(player) && !state.hasDied) {
            state.fish->appear();
            al::resetActorPosition(state.fish, player);
            //            if (rs::isOnGround(player, player->getPlayerCollision()))
            *al::getTransPtr(state.fish) += sead::Vector3f::ey * 105;
            al::setNerve(state.fish, &PukupukuNrvDoNothing::sInstance);
            player->mHackCap->prepareLockOn(state.fishSensor);
            Logger::log("detroit become fish\n");
            return;
        }

        if (isNerve<PukupukuNrvSwoon>(state.fish) || isNerve<PukupukuNrvRevive>(state.fish)) {
            Logger::log("Cutified fishy\n");
            killFishy(state);
        }

        if (!state.isFishCapture())
            return;

        unsafeRef<int>(state.fish, 0x150) = 0;

        auto* group = state.getFishEnableAreaGroup(player);
        if (group && !state.isInFishEnableArea(group, player)) {
            state.fish->endCapture();
            killFishy(state);
        }
    }
};

struct FishHandleHack : exl::hook::impl::TrampolineHook<FishHandleHack> {
    static bool Callback(Pukupuku* fish, const al::SensorMsg* msg, al::HitSensor* source, al::HitSensor* target) {
        if (stageState(fish).isFishCapture() && rs::isMsgCancelHack(msg) && StageState::isInFishEnableArea(fish)) {
            return false;
        }
        if (isNerve<PukupukuNrvDoNothing>(fish)) {
            if (rs::isMsgCapEnableLockOn(msg) || rs::isMsgCapCancelLockOn(msg))
                return true;

            if (rs::isMsgStartHack(msg)) {
                auto player = (PlayerActorHakoniwa*)al::getSensorHost(source);
                player->mPlayerAnimator->startAnim("Wait");
                player->mModelChanger->hideModel();
                player->mModelChanger->syncHost(true);
                al::offCollide(player);
                al::setVelocity(fish, al::getVelocity(player));
                player->mPlayerStateHack->mIsStageStartHack = true;
                fish->mPlayerHack = rs::startHack(target, source);
                al::invalidateClipping(fish);
                al::onCollide(fish);
                setNerveOffset(fish, 0x1ca6058); // CaptureWait
                Logger::log("Attempted to fish a success, detroit became fishy fish\n");
                return true;
            }
        }

        return Orig(fish, msg, source, target);
    }
};

struct DisableCaptureCleanupFx : exl::hook::impl::ReplaceHook<DisableCaptureCleanupFx> {
    static void Callback(PlayerStateHack* psh) {
        Logger::log("Disabling capture prepareEndHack.... %p\n", psh->mHackKeeper->mHackHitSensor);
        auto* player = psh->mActor;
        CapFunction::endHack(psh->mHackCap, psh->mAnimator);
        al::onCollide(player);
        auto& state = stageState(player);
        if (psh->mHackKeeper->getHack() != state.fish) {
            al::startHitReaction(player, "ひょうい終了");
            psh->mAnimator->startAnim("JumpEndHack");
            Logger::log("Jump end\n");
        } else {
            Logger::log("Fish end\n");
            psh->mAnimator->startAnim("Wait");
            al::setVelocity(player, al::getVelocity(state.fish));
        }
        psh->mModelChanger->showModel();
        psh->mHackKeeper->killHackDemoModel();
        al::tryDeleteEffect(player, "PossessTrace");
        setNerveOffset(psh, 0x1d7df88); // End
        Logger::log("setty images\n");
        return;
    }
};

bool fishyDeadStatus(PlayerActorHakoniwa* player) {
    return player->getPlayerHackKeeper()->getHack() == stageState(player).fish ||
           PlayerFunction::isPlayerDeadStatus(player);
}

struct ConvertNameCleanly : exl::hook::impl::TrampolineHook<ConvertNameCleanly> {
    static const char* Callback(al::AreaObjFactory* factory, const char* name) {
        if (al::isEqualString(name, "FishEnableArea"))
            return "GpuPerfArea";
        return Orig(factory, name);
    }
};

static const char* reactions[] = {"ひょうい開始", "ひょうい先に入る", "ひょうい解除失敗"};
struct DisableCertainHitReactions : exl::hook::impl::TrampolineHook<DisableCertainHitReactions> {
    static void Callback(al::HitReactionKeeper* keeper, const char* hitReactionName, const sead::Vector3f* vec,
                         const al::HitSensor* firstSensor, const al::HitSensor* secondSensor) {
        auto iter =
            std::find_if(std::begin(reactions), std::end(reactions), [hitReactionName](const char* whitelistedName) {
                return al::isEqualString(whitelistedName, hitReactionName);
            });
        if (iter == std::end(reactions))
            Orig(keeper, hitReactionName, vec, firstSensor, secondSensor);
    }
};

#pragma endregion

void fishPatches() {
    PlayerControlEnableFishy::InstallAtSymbol("_ZN19PlayerActorHakoniwa7controlEv");
    FishHandleHack::InstallAtSymbol("_ZN8Pukupuku10receiveMsgEPKN2al9SensorMsgEPNS0_9HitSensorES5_");
    DisableCaptureCleanupFx::InstallAtSymbol("_ZN15PlayerStateHack14prepareEndHackEv");
    ConvertNameCleanly::InstallAtSymbol("_ZNK2al7FactoryIPFPNS_7AreaObjEPKcEE11convertNameES4_");
    DisableCertainHitReactions::InstallAtSymbol(
        "_ZN2al17HitReactionKeeper5startEPKcPKN4sead7Vector3IfEEPKNS_9HitSensorESA_");
    patch::CodePatcher patcher(0x428988);
    patcher.BranchLinkInst((void*)fishyDeadStatus);
}

void fishDrawText(sead::TextWriter& writer) {
    writer.printf("Stateful state: %u\n", par::get<u32>("Stateful", 0));
    writer.printf("Fateful state: %s\n", par::get("Fateful", "Sussy"));
}

#pragma clang diagnostic pop
