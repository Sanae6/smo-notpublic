#include "KoopaStateRoll.hpp"
#include "al/Library/LiveActor/ActorInitInfo.h"
#include "al/Library/Nerve/NerveSetupUtil.h"
#include "al/Library/Yaml/ByamlIter.h"
#include "al/Library/Yaml/Writer/ByamlWriter.h"
#include "game/GameData/GameDataFunction.h"
#include "game/Interfaces/IUsePlayerHack.h"
#include "gfx/seadPrimitiveRenderer.h"
#include "koopa/Koopa.hpp"
#include "koopa/KoopaRoll.hpp"
#include "lib.hpp"
#include "logger/Logger.hpp"
#include "math/seadVectorCalcCommon.h"
#include "rs/util/SensorUtil.h"
#include <ExceptionHandler.h>
#include <al/Library/Controller/JoyPadUtil.h>
#include <al/Library/HitSensor/Messages.h>
#include <al/Library/LiveActor/ActorActionFunction.h>
#include <al/Library/LiveActor/ActorAnimFunction.h>
#include <al/Library/LiveActor/ActorModelFunction.h>
#include <al/Library/LiveActor/ActorMovementFunction.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/LiveActor/ActorSensorFunction.h>
#include <al/Library/Nerve/NerveStateBase.h>
#include <al/Library/Player/PlayerHolder.h>
#include <al/Library/Scene/SceneUtil.h>
#include <al/LiveActor/LiveActorFlag.hpp>
#include <game/Player/PlayerActorHakoniwa.h>
#include <rs/util.hpp>
#include <typeinfo>

#define OFFSET(ptr, offset) (((u8*)ptr) + ((size_t)offset))

namespace patch = exl::patch;
namespace inst = exl::armv8::inst;
namespace reg = exl::armv8::reg;

namespace al {
    //    class Resource {
    //    public:
    //        char* getArchiveName() const;
    //        unsigned char* tryGetByml(const sead::SafeString&) const;
    //    };
    unsigned char* tryGetByml(al::Resource const*, sead::SafeStringBase<char> const&);
    Resource* findOrCreateResource(const sead::SafeString&, const char*);
    void validateOcclusionQuery(al::LiveActor*);
    void invalidateOcclusionQuery(al::LiveActor*);
    bool tryOnStageSwitch(al::IUseStageSwitch*, char const*);
    void validateHitSensorPlayerAll(al::LiveActor*);
    void invalidateHitSensorPlayerAttackAll(al::LiveActor*);
    void showSilhouetteModelIfHide(al::LiveActor*);
    void offDepthShadowModel(al::LiveActor*);
    void invalidateDepthShadowMap(al::LiveActor*, const char*);
    void validateDepthShadowMap(al::LiveActor*, const char*);
    void resetQuatPosition(al::LiveActor*, const sead::Quatf&, const sead::Vector3f&);
    al::HitSensor* tryGetCollidedGroundSensor(al::LiveActor const*);
    const char* getMsgString(const al::SensorMsg*);
    void addVelocityJump(al::LiveActor*, float);
    float getSensorRadius(al::HitSensor const*);
    void initCreateActorNoPlacementInfo(al::LiveActor*, al::ActorInitInfo const&);
    void addDemoActor(al::LiveActor* actor);
    void startHitReaction(al::LiveActor const*, char const*);
    bool isInDeathArea(al::LiveActor const*);
    void offCollide(al::LiveActor* actor);
    bool tryKillEmitterAndParticleAll(al::IUseEffectKeeper*);

    class SensorMsg {
        SEAD_RTTI_BASE(SensorMsg);
    };
    class HitSensorKeeper {
    public:
        HitSensorKeeper(int);
        bool addSensor(al::LiveActor* sensorHost, char const* sensorName, u32 typeEnum, float radius, ushort maxCount,
                       const sead::Vector3f* position, const sead::Matrix34f* matrix, const sead::Vector3f& scale);
        void update(void);
        int getSensorNum(void) const;
        al::HitSensor* getSensor(int) const;
        void clear(void);
        void validate(void);
        void invalidate(void);
        void validateBySystem(void);
        void invalidateBySystem(void);
        al::HitSensor* getSensor(char const* sensorName) const;

        int mSensorKeeperNum;
        int mSensorNum;
        al::HitSensor** mSensors; // 0x8
    };
} // namespace al

namespace rs {
    bool tryReceiveMsgInitCapTargetAndSetCapTargetInfo(const al::SensorMsg* param_1, const CapTargetInfo* param_2);
    bool checkMsgNpcTrampleReactionAll(al::SensorMsg const*, al::HitSensor const*, al::HitSensor const*, bool);
    IUsePlayerHack* startHack(al::HitSensor*, al::HitSensor*, al::LiveActor*);
    void endHack(IUsePlayerHack**);
    bool requestDamage(IUsePlayerHack*);
    bool syncDamageVisibility(al::LiveActor*, const IUsePlayerHack*);
    bool isTouchHackDamageCollisionCode(al::LiveActor const*, IUsePlayerCollision const*);
} // namespace rs

namespace {
    MAKE_NERVE(Koopa, Bind);
    MAKE_NERVE(Koopa, BindHackStart);
    MAKE_NERVE(Koopa, Dead);
    MAKE_NERVE(Koopa, DeadFall);
    MAKE_NERVE(Koopa, Fall);
    MAKE_NERVE(Koopa, Jump);
    MAKE_NERVE(Koopa, Start);
    MAKE_NERVE(Koopa, Swoon);
    MAKE_NERVE(Koopa, SwoonReaction);
    MAKE_NERVE(Koopa, Wait);
    MAKE_NERVE(Koopa, 2D);
    MAKE_NERVE(Koopa, Roll);
    MAKE_NERVE(PlayerActorHakoniwa, Hack);
    class KoopaNrvWaitReset : public al::Nerve {
        void execute(al::NerveKeeper* keeper) const override {}
    };
    KoopaNrvWaitReset nrvKoopaWaitReset;
} // namespace

struct PlayerStateHack : public al::ActorStateBase {
    PlayerHackKeeper* hackKeeper;
    PlayerModelChangerHakoniwa changer;
    PlayerAnimator* animator;
    HackCap* cap;
    bool startsHacked;
    void prepareStageStartHack();
};

bool isNerve(al::IUseNerve* nerveUser, const al::Nerve* nerve) {
    return al::isEqualString(typeid(*nerveUser->getNerveKeeper()->getCurrentNerve()).name(), typeid(*nerve).name());
}

al::PlacementInfo placementInfo;
static al::PlacementInfo* setupKoopaPlacementInfo(al::Resource* res) {
    al::ByamlIter iter(res->tryGetByml("Bowser"));
    placementInfo.set(iter, iter);
    return &placementInfo;
}

int demoage = 0;
bool wasHacked = false;

void handleKoopaHack(Koopa* koopa) {
    al::invalidateClipping(koopa);
    al::invalidateOcclusionQuery(koopa);
    al::tryOnStageSwitch(koopa, "SwitchRestartOn");
    koopa->mKoopaPartialAnimator->appear();
    // al::invalidateHitSensors(koopa);
    // al::validateHitSensorPlayerAll(koopa);
    // al::invalidateHitSensorPlayerAttackAll(koopa);
    al::startVisAnim(koopa, "HackOff"); // no mario!!!!
    al::startAction(koopa, "Wait");
    al::clearSklAnimInterpole(koopa);
    GameDataFunction::getLifeMaxUpItem(koopa);
    al::showSilhouetteModelIfHide(koopa);
    al::offDepthShadowModel(koopa);
    al::invalidateDepthShadowMap(koopa, "Body");
    al::validateDepthShadowMap(koopa, "BodyHack");
    al::validateDepthShadowMap(koopa, "GroundHack");
    al::tryOnStageSwitch(koopa, "SwitchAfterPlayableOn");
    al::setNerve(koopa, &nrvKoopaWait);
    wasHacked = false;
}

class ShineTowerRocket : al::LiveActor {};

void setSecretTwelfthFlag(al::LiveActor* actor, bool value) {
    auto* flags = actor->getFlags();
    bool& secretTwelthFlag = flags->mSecretTwelfthFlagHehe;
    if (secretTwelthFlag != value) {
        secretTwelthFlag = value;
        if (value) {
            al::hideModel(actor);
            flags->mClipped = true;
            flags->mNoCollide = true;
            flags->mUpdateMovementEffectAudioCollisionSensor = false;
            Logger::log("Made actor eepy!!!\n");
        } else {
            flags->mClipped = false;
            flags->mNoCollide = false;
            flags->mUpdateMovementEffectAudioCollisionSensor = true;
            al::showModel(actor);
        }
    }
}

struct KoopaReceiveMsg : public exl::hook::impl::ReplaceHook<KoopaReceiveMsg> {
    static bool Callback(Koopa* koopa, const al::SensorMsg* msg, al::HitSensor* source, al::HitSensor* target) {
        if (al::getSensorHost(target) != koopa ||
            typeid(*al::getSensorHost(target)).hash_code() == typeid(ShineTowerRocket).hash_code())
            return false;
        if (isNerve(koopa, &nrvKoopaDead) || isNerve(koopa, &nrvKoopaDeadFall))
            return false;
        if (rs::tryReceiveMsgInitCapTargetAndSetCapTargetInfo(msg, koopa->mCapTargetInfo)) {
            Logger::log("Set cap target info\n");
            return true;
        }

        if (rs::isMsgKoopaBindStart(msg))
            return !isNerve(koopa, &nrvKoopaBind) && !isNerve(koopa, &nrvKoopa2D);

        if (al::isMsgPlayerDisregard(msg))
            return true;

        if (rs::isMsgCapEnableLockOn(msg))
            return isNerve(koopa, &nrvKoopaSwoon) || isNerve(koopa, &nrvKoopaSwoonReaction) ||
                   isNerve(koopa, &nrvKoopaWaitReset);
        if (rs::isMsgCapCancelLockOn(msg))
            return true;
        IUsePlayerHack** hack = &koopa->mPlayerHack;
        if (rs::isMsgHackDirectStageInit(hack, msg)) {
            auto* player = al::getPlayerActor(koopa, 0);
            al::resetQuatPosition(koopa, al::getQuat(player), al::getTrans(player));
            handleKoopaHack(koopa);
            return true;
        }

        if (koopa->mPlayerHack == nullptr && rs::checkMsgNpcTrampleReactionAll(msg, source, target, false) &&
            !isNerve(koopa, &nrvKoopaWaitReset)) {
            rs::requestHitReactionToAttacker(msg, target, source);
            al::setNerve(koopa, &nrvKoopaSwoonReaction);
            return true;
        }

        if (rs::isMsgStartHack(msg)) {
            Logger::log("Tried to start hack\n");
            *hack = rs::startHack(target, source, nullptr);
            handleKoopaHack(koopa);
            return true;
        }

        if (rs::isMsgHackMarioCheckpointFlagWarp(msg)) {
            al::startVisAnim(koopa, "HackOff");
            return true;
        }
        if (rs::isMsgHackMarioInWater(msg))
            return true;
        if (rs::isMsgHackInvalidEscapeNoReaction(msg) || rs::isMsgHackInvalidEscape(msg) ||
            rs::isMsgNpcScareByEnemy(msg))
            return false;

        if (rs::isMsgHackMarioDemo(msg))
            return true;

        if (rs::isMsgHackDemoStart(msg)) {
            Logger::log("Demo started!\n");
            auto* player = (PlayerActorHakoniwa*)al::getPlayerActor(koopa, 0);
            al::addDemoActor(koopa);
            al::invalidateDepthShadowMap(koopa, "BodyHack");
            al::invalidateDepthShadowMap(koopa, "GroundHack");
            al::startAction(koopa, "Wait");
            al::startHitReaction(koopa, "死亡");
            al::setNerve(koopa, &nrvKoopaWaitReset);
            al::updatePoseTrans(koopa, al::getTrans(player));
            al::resetPosition(koopa);
            al::validateOcclusionQuery(koopa);
            al::setVelocityZero(koopa);
            al::invalidateHitSensors(koopa);
            setSecretTwelfthFlag(koopa, true);
            rs::endHack(&koopa->mPlayerHack);
            koopa->mPlayerHack = nullptr;
//            demoage = 20;
            return true;
        }

        if (rs::isMsgHackDemoEnd(msg)) {
            Logger::log("Demo ended");
            auto* player = (PlayerActorHakoniwa*)al::getPlayerActor(koopa, 0);
            al::updatePoseTrans(koopa, al::getTrans(player));
            al::resetPosition(koopa);


//            al::invalidateOcclusionQuery(koopa);
//            setSecretTwelfthFlag(koopa, false);
//            al::setNerve(koopa, &nrvKoopaWait);
//            demoage = 20;
            return true;
        }

        if (rs::isMsgHackMarioDead(msg)) {
            al::validateOcclusionQuery(koopa);
            al::startVisAnim(koopa, "HackOff");
            al::setNerve(koopa, al::isInDeathArea(koopa) ? (al::Nerve*)&nrvKoopaDeadFall : (al::Nerve*)&nrvKoopaDead);
            return true;
        }

        if (!koopa->getFlags()->mSecretTwelfthFlagHehe && demoage == 0 && !al::isActiveDemo(koopa->scene) &&
            (rs::isMsgCancelHack(msg) || rs::isMsgKillByHomeDemo(msg) || rs::isMsgCancelHackByDokan(msg))) {
            Logger::log("Cancelled hack: %s (%s) from %s\n", typeid(*msg).name(), typeid(msg).name(),
                        typeid(*source->host).name());
            //            handler::tryCatch(
            //                []() {
            //                    (*(int*)0x8) = 5;
            //                },
            //                [](handler::ExceptionInfo& info) {
            //                    handler::printStackTrace(info);
            //                    return true;
            //                });
            rs::endHack(hack);
            koopa->mPlayerHack = nullptr;
            al::validateOcclusionQuery(koopa);
            al::startVisAnim(koopa, "HackOff");

            const al::Nerve* nextNerve =
                rs::isMsgHackMarioDead(msg)
                    ? al::isInDeathArea(koopa) ? (al::Nerve*)&nrvKoopaDeadFall : (al::Nerve*)&nrvKoopaDead
                    : (al::Nerve*)&nrvKoopaWaitReset; // was swoon, trying it out
            al::setNerve(koopa, nextNerve);
            if (nextNerve == &nrvKoopaWaitReset) {
                al::setVelocityZero(koopa);
                al::validateHitSensors(koopa);
                al::invalidateOcclusionQuery(koopa);
                setSecretTwelfthFlag(koopa, true);
            }
            return true;
        }

        if (al::isMsgEnemyAttack(msg)) {
            return rs::sendMsgCapAttack(source, target);
        }

        if (koopa->mPlayerHack != nullptr && (rs::isMsgKoopaHackDamage(msg) || rs::isMsgHackerDamageAndCancel(msg))) {
            return rs::requestDamage(*hack);
        }

        if (koopa->mPlayerHack != nullptr && rs::isMsgHackSyncDamageVisibility(msg)) {
            if (isNerve(koopa, &nrvKoopaBind) || isNerve(koopa, &nrvKoopaBindHackStart))
                return false;

            rs::syncDamageVisibility(koopa, koopa->mPlayerHack);
            return true;
        }

        //        Logger::log("Got unhandled message: %s (%s) from %s\n", typeid(*msg).name(), typeid(msg).name(),
        //                    typeid(*source->host).name());

        return false;
    }
};

bool koopaAttackSensorPlus(Koopa* koopa, al::HitSensor* target, al::HitSensor* source) {
    if (al::sendMsgPlayerItemGet(source, target) || rs::sendMsgShineGet(source, target))
        return true;
    if (al::sendMsgPlayerAttackTrample(source, target, nullptr))
        return true;
    if (al::sendMsgPlayerHipDrop(source, target, nullptr))
        return true;
    if (rs::sendMsgCapStartLockOn(source, target))
        return true;

    if ((rs::sendMsgSenobiCancelStretch(source, target) || rs::sendMsgHackUpperPunch(source, target) ||
         rs::sendMsgKoopaHackPunch(source, target) || rs::sendMsgTankBullet(source, target) ||
         rs::sendMsgBossKnuckleKillerAttack(source, target) ||
         al::sendMsgPlayerAttackTrample(source, target, nullptr) || al::sendMsgPlayerHipDrop(source, target, nullptr) ||
         rs::sendMsgCapAttack(source, target) || rs::sendMsgCapReflect(source, target) || rs::sendMsgWanwanReboundAttack(source, target))) {
        return true;
    }
    if (al::sendMsgPlayerTrampleReflect(source, target, nullptr)) {
        al::addVelocityJump(koopa, 10);
        return true;
    }

    return false;
}

struct KoopaAttackSensor : public exl::hook::impl::ReplaceHook<KoopaAttackSensor> {
    static void Callback(Koopa* koopa, al::HitSensor* target, al::HitSensor* source) {
        if (isNerve(koopa, &nrvKoopaDead) || isNerve(koopa, &nrvKoopaDeadFall))
            return;

        if (al::isSensorEnemyAttack(target)) {

            //            Logger::log("Attacking: %s %s With: \n", source->name,
            //            typeid(*al::getSensorHost(source)).name(), target->name);
            rs::sendMsgPushToPlayer(source, target);
            return;
        }

        if (koopa->mPlayerHack == nullptr)
            return;

        if (koopaAttackSensorPlus(koopa, target, source))
            return;

        if (isNerve(koopa, &nrvKoopaStart) && rs::sendMsgHackerNoReaction(koopa->mPlayerHack, source, target))
            return;

        if (isNerve(koopa, &nrvKoopaStart) || isNerve(koopa, &nrvKoopaJump) || isNerve(koopa, &nrvKoopaFall)) {
            const sead::Vector3f& gravity = al::getGravity(koopa);
            const sead::Vector3f& velocity = al::getVelocity(koopa);

            if (0 < sead::Vector3CalcCommon<float>::dot(gravity, velocity) &&
                rs::sendMsgKoopaHackTrample(source, target))
                return;
        }

        koopa->mKoopaPartialAnimator->attackSensor(target, source);
    }
};

extern "C" void _ZN2al9LiveActor8movementEv(al::LiveActor*);
extern "C" void _ZN2al9LiveActor14updateColliderEv(al::LiveActor*);
bool inDemo = false;

const char* getPlayerHackPtr() { return typeid(nrvPlayerActorHakoniwaHack).name(); }
const al::Nerve* getPlayerHack() { return &nrvPlayerActorHakoniwaHack; }

struct KoopaMovementWrapper : public exl::hook::impl::TrampolineHook<KoopaMovementWrapper> {
    static void Callback(Koopa* koopa) {
        al::LiveActorFlag* flags = koopa->getFlags();

        auto* player = (PlayerActorHakoniwa*)al::getPlayerActor(koopa, 0);
        if (demoage > 0) {
            demoage--;
            Logger::log("Demoage test %d\n", demoage, BTOC(koopa->mPlayerHack));
            if (demoage == 0 && koopa->mPlayerHack) {
                Logger::log("eeeyhaaahhaha\n");
                al::offCollide(player);
                al::setVelocityZero(player);
                player->mModelChanger->hideModel();
                player->mModelChanger->syncHost(true);
                al::tryKillEmitterAndParticleAll(player);
                *(CapTargetInfo**)OFFSET(player->mHackCap, 0x228) = koopa->mCapTargetInfo;
                player->mPlayerStateHack->startsHacked = true;
                al::setNerve(player, &nrvPlayerActorHakoniwaHack);
            }
        }

        if (!flags->mSecretTwelfthFlagHehe) {
            Orig(koopa);
            return;
        }

        if (koopa->mPlayerHack)
            *(CapTargetInfo**)OFFSET(player->mHackCap, 0x228) = koopa->mCapTargetInfo;

        if (al::isPadTriggerLeft(-1) && !al::isActiveDemo(koopa->scene) &&
            !isNerve(player, &nrvPlayerActorHakoniwaHack) && !rs::isPlayer2D(player)) {
            Logger::log("Tried restarting bowser\n");
            al::updatePoseTrans(koopa, al::getTrans(player));
            al::resetPosition(koopa);
            al::invalidateOcclusionQuery(koopa);
            setSecretTwelfthFlag(koopa, false);
            auto* head = al::getHitSensor(koopa, "SwoonHead");
            player->mHackCap->prepareLockOn(head);
        }
        _ZN2al9LiveActor8movementEv(koopa);
    }
};

struct KoopaControlWrapper : public exl::hook::impl::TrampolineHook<KoopaControlWrapper> {
    static void Callback(Koopa* koopa) {
        if (!al::isActiveDemo(koopa->scene)) {
            if (koopa->mPlayerHack && rs::isTouchHackDamageCollisionCode(koopa, nullptr))
                rs::requestDamage(koopa->mPlayerHack);
            Orig(koopa);
        }
    }
};

void koopaMovement(Koopa* koopa) {
    // todo: floor code jump!!!!!
    al::setScaleAll(koopa, 0.75);

    _ZN2al9LiveActor8movementEv(koopa);
}

void koopaUpdateCollider(Koopa* koopa) {
    sead::Vector3f finalVel, curVel = al::getVelocity(koopa);
    koopa->playerPushReceiver->calcPushedVelocity(&finalVel, al::getVelocity(koopa));
    al::setVelocity(koopa, finalVel);
    _ZN2al9LiveActor14updateColliderEv(koopa);
    al::setVelocity(koopa, curVel);
    koopa->playerPushReceiver->clear();
}

Koopa* koopa = nullptr;

void koopaInit(al::Scene* scene, const al::ActorInitInfo& initInfo, char* listName) {
    al::initPlacementObjectMap(scene, initInfo, listName);

    al::ActorInitInfo koopaInitInfo(initInfo);

    auto* res = al::findOrCreateResource("SystemData/KoopaPlacement", nullptr);
    koopaInitInfo.initViewIdSelf(setupKoopaPlacementInfo(res), initInfo);
    koopa = new Koopa("KoopaHack");
    koopa->playerPushReceiver = new PlayerPushReceiver(koopa);
    koopa->scene = scene;

    al::initCreateActorWithPlacementInfo(koopa, koopaInitInfo);
    koopa->mCapTargetInfo->setHackName("Koopa");
    al::setTrans(koopa, al::getTrans(al::getPlayerActor(koopa, 0)));
    al::updatePoseQuat(koopa, al::getQuat(al::getPlayerActor(koopa, 0)));
}

void koopaDrawDebug(sead::PrimitiveRenderer& renderer) {
    if (koopa == nullptr)
        return;
    for (int i = 0; i < koopa->getHitSensorKeeper()->getSensorNum(); i++) {
        auto* sensor = koopa->getHitSensorKeeper()->getSensor(i);
        renderer.drawSphere4x8(al::getSensorPos(sensor), al::getSensorRadius(sensor), sead::Color4f::cRed);
    }
}

bool is2DNerve(const al::IUseNerve* user, const al::Nerve* nerve) {
    return al::isNerve(user, nerve) || isNerve(user, &nrvKoopaRoll);
}

sead::Quatf* getQuatPtrKoopa(al::LiveActor* actor) { return al::getQuatPtr(actor); }

struct DemoStartHandler : public exl::hook::impl::TrampolineHook<DemoStartHandler> {
    static bool Callback(al::DemoDirector* demoDirector, const char* name, int demoType) {
        bool res = Orig(demoDirector, name, demoType);
        if (res) Logger::log("Started demo\n");
        if (res && koopa && koopa->mPlayerHack) {
            rs::sendMsgHackDemoStart(koopa->mHitSensorKeeper->getSensor(0), koopa->mHitSensorKeeper->getSensor(0));
        }
        return res;
    }
};

struct DemoEndHandler : public exl::hook::impl::TrampolineHook<DemoEndHandler> {
    static void Callback(al::DemoDirector* demoDirector, const char* name, int demoType) {
        Orig(demoDirector, name, demoType);
        Logger::log("Ended demo\n");
        if (koopa && koopa->mPlayerHack)
            rs::sendMsgHackDemoEnd(koopa->mHitSensorKeeper->getSensor(0), koopa->mHitSensorKeeper->getSensor(0));
    }
};

struct EndInitHandler : public exl::hook::impl::TrampolineHook<EndInitHandler> {
    static void Callback(al::Scene* scene) {
        Orig(scene);
        koopa = nullptr;
        demoage = 0;
        wasHacked = false;
    }
};

void koopaSideInit(patch::CodePatcher& patcher);
void koopaPatchesInit() {
    patch::CodePatcher p(0x4ca0e4);
    p.BranchLinkInst((void*)&koopaInit); // setup init
    p.Seek(0x7de80);                     // todo: create global camera ticket
    p.WriteInst(inst::Nop());
    p.WriteInst(inst::Nop());
    p.Seek(0x4cfeb4); //
    p.WriteInst(inst::Movz(reg::X0, 0x1));
    p.Seek(0x550520);
    p.WriteInst(inst::Ret());
    p.Seek(0x45ade4);
    p.WriteInst(inst::MovRegister(reg::X0, reg::None64));
    p.Seek(0x3de4d4);
    p.WriteInst(inst::MovRegister(reg::X0, reg::None64));
    p.Seek(0x7e7c0);
    p.WriteInst(inst::Nop());
    p.Seek(0x4cb4b8);
    p.WriteInst(inst::MovRegister(reg::X0, reg::None64));
    p.Seek(0x4cb4dc);
    p.WriteInst(inst::Movz(reg::X0, 0x1));
    p.Seek(0x4b92fc); // resize Koopa creator memory to 0x230 from 0x220
    p.WriteInst(inst::Movz(reg::X0, 0x230));
    p.Seek(0x7ea70); // updateCollider for player push receiver
    p.BranchLinkInst((void*)&koopaUpdateCollider);
    p.Seek(0x7e6a0); // movement for switch to roll
    p.BranchLinkInst((void*)&koopaMovement);
    p.Seek(0x7da3c); // change max nerve state count to 3
    p.WriteInst(inst::Movz(reg::W2, 0x3));
    p.Seek(0x3f4200); // disable effect thingy
    p.WriteInst(inst::Nop());
    //    p.Seek(0x7e7dc);
    //    p.BranchLinkInst((void*)&is2DNerve);
    //    p.Seek(0x3f4b48);
    //    p.BranchLinkInst((void*)&getQuatPtrKoopa);
    p.Seek(0x3f4d08);
    p.WriteInst(inst::MovRegister(reg::W8, reg::None32));
    p.Seek(0x3f4b3c);
    p.WriteInst(inst::Movz(reg::X0, 1));
    p.Seek(0x7fc14);
    p.WriteInst(inst::Movz(reg::W1, 16));
    //    p.Seek(0x4273f4); // Disable recovery
    //    p.WriteInst(inst::Ret());
    //    p.Seek(0x4202bc); // Disable if in demo
    //    p.WriteInst(inst::Movz(reg::X0, 0));
    //    p.Seek(0x4202f0); // we're in an active demo always
    //    p.WriteInst(inst::Movz(reg::X0, 0));
    //    p.Seek(0x420310); // isInRecovery: false
    //    p.WriteInst(inst::Movz(reg::X0, 0));
    p.Seek(0x420294);
    p.WriteInst(inst::Movz(reg::X0, 0));
    p.Seek(0x1f2998);
    p.WriteInst(inst::Movz(reg::X0, 1));
    p.WriteInst(inst::Ret());
    koopaSideInit(p);

    KoopaReceiveMsg::InstallAtOffset(0x7ed3c);
    KoopaAttackSensor::InstallAtOffset(0x7ec00);
    KoopaControlWrapper::InstallAtSymbol("_ZN5Koopa7controlEv");
    KoopaMovementWrapper::InstallAtSymbol("_ZN5Koopa8movementEv");
    DemoStartHandler::InstallAtSymbol("_ZN19ProjectDemoDirector16requestStartDemoEPKcNS_8DemoTypeE");
    DemoEndHandler::InstallAtSymbol("_ZN19ProjectDemoDirector14requestEndDemoEPKcNS_8DemoTypeE");
    EndInitHandler::InstallAtSymbol("_ZN2al5Scene4killEv");
}
