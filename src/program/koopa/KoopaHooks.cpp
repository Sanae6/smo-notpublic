#include "AmongWriteStream.h"
#include "KoopaStateRoll.hpp"
#include "al/Library/LiveActor/ActorClippingFunction.h"
#include "al/Library/LiveActor/ActorInitInfo.h"
#include "al/Library/LiveActor/LiveActor.h"
#include "al/Library/Nerve/NerveSetupUtil.h"
#include "al/Library/Yaml/ByamlIter.h"
#include "al/Library/Yaml/Writer/ByamlWriter.h"
#include "al/Project/HitSensor/HitSensor.h"
#include "game/GameData/GameDataFunction.h"
#include "game/Interfaces/IUsePlayerHack.h"
#include "gfx/seadPrimitiveRenderer.h"
#include "imgui.h"
#include "imgui_nvn.h"
#include "koopa/Koopa.hpp"
#include "koopa/KoopaRoll.hpp"
#include "lib.hpp"
#include "logger/Logger.hpp"
#include "math/seadVector.h"
#include "math/seadVectorCalcCommon.h"
#include "nx/kernel/svc.h"
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
#include <math/seadQuat.h>
#include <rs/util.hpp>
#include <typeinfo>

#define OFFSET(ptr, offset) (((u8*)ptr) + ((size_t)offset))
#define OFFSET_CAST(ptr, offset, type) ((type)(OFFSET(ptr, offset)))

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
    void requestStageStartHack(const al::LiveActor* self, al::HitSensor* sensor, const CapTargetInfo* info,
                               al::LiveActor* unused);
    bool sendMsgStartHack(al::HitSensor*, al::HitSensor*);
    bool isActiveDemo(const al::LiveActor* actor);
} // namespace rs

struct PlayerStartObj : al::LiveActor {
    void exeDemoWait();
    void exeBindEnd();
};

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
    MAKE_NERVE(HackCap, LockOn);
    MAKE_NERVE(PlayerStartObj, DemoWait);
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

bool haveIBeenPwned = true;
int demoage = 0;
const int reactedCreatureCount = 50;
struct ReactedCreature {
    int timer = -1;
    al::LiveActor* actor = nullptr;
} reactedCreatures[reactedCreatureCount] = {};

void addReacted(al::LiveActor* actor, int newTime) {
    // bool failed = true;
    for (int i = 0; i < reactedCreatureCount; i++) {
        ReactedCreature& creature = reactedCreatures[i];

        // if (creature.actor != nullptr) {
        //     Logger::log("%d - New: %s (%p), Old: %s (%p)\n", i, actor, typeid(*actor).name(), creature.actor,
        //     typeid(*creature.actor).name());
        // }
        if (creature.actor == actor || creature.timer == -1) {
            creature.timer = newTime;
            creature.actor = actor;
            // failed = false;
            Logger::log("Added reacted creature %s with time %d at %d\n", typeid(*creature.actor).name(), newTime, i);
            // svcSleepThread(1000000000);
            break;
        }
    }
    // if (failed) {
    //     Logger::log("Warn: failed to add actor %s with time %d\n", typeid(*actor).name(), newTime);
    //     svcSleepThread(1000000000);
    // }
}

bool isReacted(al::LiveActor* actor) {
    for (const auto& item : reactedCreatures) {
        if (item.actor == actor) {
            return item.timer > 0;
        }
    }
    return false;
}

void tickReacted() {
    for (auto& creature : reactedCreatures) {
        if (creature.timer >= 0)
            creature.timer--;
    }
}

void setSecretTwelfthFlag(al::LiveActor* actor, bool value) {
    auto* flags = actor->getFlags();
    bool& secretTwelthFlag = flags->mSecretTwelfthFlagHehe;
    if (secretTwelthFlag != value) {
        secretTwelthFlag = value;
        if (value) {
            al::hideModel(actor);
            flags->mClipped = false;
            flags->mNoCollide = true;
            flags->mUpdateMovementEffectAudioCollisionSensor = false;
            al::invalidateDepthShadowMap(actor, "BodyHack");
            al::invalidateDepthShadowMap(actor, "GroundHack");
            Logger::log("Made actor eepy!!!\n");
        } else {
            flags->mClipped = false;
            flags->mNoCollide = false;
            flags->mUpdateMovementEffectAudioCollisionSensor = true;
            al::showModel(actor);
        }
    }
}

void handleKoopaHack(Koopa* koopa) {
    setSecretTwelfthFlag(koopa, false);
    al::invalidateClipping(koopa);
    al::invalidateOcclusionQuery(koopa);
    al::tryOnStageSwitch(koopa, "SwitchRestartOn");
    koopa->mKoopaPartialAnimator->clear();
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
    haveIBeenPwned = true;
    auto* player = (PlayerActorHakoniwa*)al::getPlayerActor(koopa, 0);
    player->mModelChanger->hideModel();
    player->mModelChanger->syncHost(true);
    Logger::log("Hack started!\n");
}

class ShineTowerRocket : al::LiveActor {};
class KoopaFireBall : al::LiveActor {};
class KoopaLv1 : public al::LiveActor {};
class MofumofuBody : public al::LiveActor {};
bool koopaAttackSensorPlus(al::LiveActor*, al::HitSensor*, al::HitSensor*);

static int DemoCounter;
struct KoopaReceiveMsg : public exl::hook::impl::ReplaceHook<KoopaReceiveMsg> {
    static bool Callback(Koopa* koopa, const al::SensorMsg* msg, al::HitSensor* source, al::HitSensor* target) {
        if (al::getSensorHost(target) != koopa ||
            typeid(*al::getSensorHost(target)).hash_code() == typeid(ShineTowerRocket).hash_code() ||
            typeid(*al::getSensorHost(target)).hash_code() == typeid(KoopaFireBall).hash_code())
            return false;
        //        if (al::getSensorHost(target) != koopa ||
        //            typeid(*al::getSensorHost(target)).hash_code() == typeid(ShineTowerRocket).hash_code())
        //            return false;
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
            Logger::log("Tried to stage start hack\n");
            auto* player = al::getPlayerActor(koopa, 0);
            al::resetQuatPosition(koopa, al::getQuat(player), al::getTrans(player));
            al::setGravity(koopa, -sead::Vector3f::ey);
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
            al::updatePoseTrans(koopa, al::getTrans(player));
            al::resetQuatPosition(koopa, al::getQuat(player), al::getTrans(player));
            al::setGravity(koopa, -sead::Vector3f::ey);
            al::validateOcclusionQuery(koopa);
            al::setVelocityZero(koopa);
            al::invalidateHitSensors(koopa);
            setSecretTwelfthFlag(koopa, true);
            // demo end hacks don't count, haveIBeenPwned = true still
            rs::endHack(&koopa->mPlayerHack);
            koopa->mPlayerHack = nullptr;
            al::tryKillEmitterAndParticleAll(koopa);
            al::setNerve(koopa, &nrvKoopaWaitReset);
            DemoCounter = 20;
            return true;
        }

        //        if (rs::isMsgHackDemoEnd(msg)) {
        //            Logger::log("Demo ended");
        //            auto* player = (PlayerActorHakoniwa*)al::getPlayerActor(koopa, 0);
        //            al::updatePoseTrans(koopa, al::getTrans(player));
        //            al::resetPosition(koopa);
        //            return true;
        //        }

        if (rs::isMsgHackMarioDead(msg)) {
            al::validateOcclusionQuery(koopa);
            al::startVisAnim(koopa, "HackOff");
            al::setNerve(koopa, al::isInDeathArea(koopa) ? (al::Nerve*)&nrvKoopaDeadFall : (al::Nerve*)&nrvKoopaDead);
            return true;
        }

        if (!koopa->getFlags()->mSecretTwelfthFlagHehe && demoage == 0 && !rs::isActiveDemo(koopa) &&
            (rs::isMsgCancelHack(msg) || rs::isMsgKillByHomeDemo(msg) || rs::isMsgCancelHackByDokan(msg) ||
             rs::isMsgTransferHack(msg))) {
            Logger::log("Cancelled hack: %s (%s) from %s\n", typeid(*msg).name(), typeid(msg).name(),
                        typeid(*source->host).name());
            rs::endHack(hack);
            koopa->mPlayerHack = nullptr;
            haveIBeenPwned = false;
            al::validateOcclusionQuery(koopa);
            al::startVisAnim(koopa, "HackOff");
            al::tryKillEmitterAndParticleAll(koopa);

            const al::Nerve* nextNerve =
                rs::isMsgHackMarioDead(msg)
                    ? al::isInDeathArea(koopa) ? (al::Nerve*)&nrvKoopaDeadFall : (al::Nerve*)&nrvKoopaDead
                    : (al::Nerve*)&nrvKoopaWaitReset; // was swoon, trying it out
            al::setNerve(koopa, nextNerve);
            if (nextNerve == &nrvKoopaWaitReset) {
                al::setVelocityZero(koopa);
                al::invalidateHitSensors(koopa);
                al::invalidateOcclusionQuery(koopa);
                setSecretTwelfthFlag(koopa, true);
            }
            return true;
        }

        bool anyAttacked =
            al::isMsgEnemyAttack(msg) || rs::isMsgKoopaHackDamage(msg) || rs::isMsgHackerDamageAndCancel(msg);

        if (anyAttacked && !al::isEqualString(typeid(*source->host).name(), typeid(KoopaLv1).name()) &&
            !al::isEqualString(typeid(*source->host).name(), typeid(MofumofuBody).name())) {
            return koopaAttackSensorPlus(koopa, target, source) ||
                   (hack != nullptr && *hack && rs::requestDamage(*hack));
        }

        if (koopa->mPlayerHack != nullptr && rs::isMsgHackSyncDamageVisibility(msg)) {
            if (isNerve(koopa, &nrvKoopaBind) || isNerve(koopa, &nrvKoopaBindHackStart))
                return false;

            rs::syncDamageVisibility(koopa, koopa->mPlayerHack);
            return true;
        }

        // if (!al::isEqualString(typeid(*source->host).name(), typeid(PlayerActorHakoniwa).name()))
        //     Logger::log("Got unhandled message: %s (%s) from %s\n", typeid(*msg).name(), typeid(msg).name(),
        //                 typeid(*source->host).name());

        return false;
    }
};

bool koopaPunch(al::HitSensor* target, al::HitSensor* source, int& recTimer);
bool koopaAttackSensorPlus(al::LiveActor* actor, al::HitSensor* target, al::HitSensor* source) {
    if (al::isEqualString(typeid(*source->host).name(), typeid(Koopa).name()) ||
        al::isEqualString(typeid(*source->host).name(), typeid(KoopaFireBall).name()))
        return false;
    if (isReacted(source->host))
        return true;
    if (rs::sendMsgTRexAttackAll(source, target)) {
        return true;
    }
    if (rs::sendMsgWanwanEnemyAttack(source, target) || rs::sendMsgWanwanReboundAttack(source, target)) {
        addReacted(source->host, 120);
        Logger::log("Attacked wanwan!\n");
        return true;
    }
    if (al::sendMsgPlayerItemGet(source, target) || rs::sendMsgShineGet(source, target))
        return true;
    if (rs::sendMsgCapStartLockOn(source, target))
        return true;

    if (rs::sendMsgCapTouchWall(source, target, al::getSensorPos(target), al::getSensorPos(target)) ||
        al::sendMsgPlayerTouch(source, target) || rs::sendMsgCapHipDrop(source, target) ||
        rs::sendMsgSenobiCancelStretch(source, target) || rs::sendMsgHackUpperPunch(source, target) ||
        rs::sendMsgKoopaHackPunch(source, target) || rs::sendMsgTankBullet(source, target) ||
        rs::sendMsgBossKnuckleKillerAttack(source, target) || al::sendMsgPlayerAttackTrample(source, target, nullptr) ||
        al::sendMsgPlayerHipDrop(source, target, nullptr) || rs::sendMsgCapAttack(source, target) ||
        rs::sendMsgCapReflect(source, target) || rs::sendMsgBossKnuckleKillerAttack(source, target)) {
        addReacted(source->host, 30);
        return true;
    }
    if (al::sendMsgPlayerTrampleReflect(source, target, nullptr)) {
        al::addVelocityJump(actor, 10);
        return true;
    }

    return false;
}

Koopa* koopa = nullptr;

struct FireballAttackSensor : public exl::hook::impl::TrampolineHook<FireballAttackSensor> {
    static void Callback(al::LiveActor* fireball, al::HitSensor* target, al::HitSensor* source) {
        if (source->host == koopa)
            return;

        if (al::sendMsgEnemyAttackFire(source, target, nullptr) || al::sendMsgPlayerFireBallAttack(source, target))
            return;
        if (koopaAttackSensorPlus(fireball, target, source))
            return;

        Orig(fireball, target, source);
    }
};
bool punchL(al::HitSensor* sensor);
bool punchR(al::HitSensor* sensor);
struct KoopaAttackSensor : public exl::hook::impl::ReplaceHook<KoopaAttackSensor> {
    static void Callback(Koopa* koopa, al::HitSensor* target, al::HitSensor* source) {
        if (isNerve(koopa, &nrvKoopaDead) || isNerve(koopa, &nrvKoopaDeadFall))
            return;

        int recTimer = 0;
        if (al::isEqualString(typeid(*source->host).name(), typeid(KoopaLv1).name())) {
            if ((punchL(target) || punchR(target)) && !isReacted(source->host) &&
                koopaPunch(target, source, recTimer)) {
                addReacted(source->host, recTimer);
                Logger::log("Rec timer: %d", recTimer);
            }
            return;
        }

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

const char* getPlayerHackPtr() { return typeid(nrvPlayerActorHakoniwaHack).name(); }
const al::Nerve* getPlayerHack() { return &nrvPlayerActorHakoniwaHack; }

struct KoopaMovementWrapper : public exl::hook::impl::TrampolineHook<KoopaMovementWrapper> {
    static void Callback(Koopa* koopa) {
        al::LiveActorFlag* flags = koopa->getFlags();

        auto* player = (PlayerActorHakoniwa*)al::getPlayerActor(koopa, 0);

        tickReacted();
        if (DemoCounter > 0) {
            DemoCounter--;
            if (rs::isActiveDemo(koopa)) {
                DemoCounter = 20;
            }
            if (DemoCounter == 0) {
                if (!rs::isPlayer2D(player) && !isNerve(player, &nrvPlayerActorHakoniwaHack)) {
                    Logger::log("Tried restarting bowser\n");
                    al::updatePoseTrans(koopa, al::getTrans(player));
                    al::resetQuatPosition(koopa, al::getQuat(player), al::getTrans(player));
                    al::setGravity(koopa, -sead::Vector3f::ey);
                    al::validateHitSensors(koopa);
                    al::invalidateOcclusionQuery(koopa);
                    // setSecretTwelfthFlag(koopa, false);
                    auto* head = al::getHitSensor(koopa, "SwoonHead");
                    player->mHackCap->prepareLockOn(head);
                }
            }
        }

        if (koopa->mPlayerHack) {
            if (!isNerve(player, &nrvPlayerActorHakoniwaHack)) {
                rs::endHack(&koopa->mPlayerHack);
                koopa->mPlayerHack = nullptr;
                haveIBeenPwned = false;
                al::validateOcclusionQuery(koopa);
                al::startVisAnim(koopa, "HackOff");
                al::tryKillEmitterAndParticleAll(koopa);
                al::setVelocityZero(koopa);
                al::invalidateHitSensors(koopa);
                al::invalidateOcclusionQuery(koopa);
                setSecretTwelfthFlag(koopa, true);
                al::setNerve(koopa, &nrvKoopaWaitReset);
                _ZN2al9LiveActor8movementEv(koopa);
                return;
            }
            player->mModelChanger->update(true, false);
        }
        if (al::isPadTriggerLeft(-1)) {
            if (flags->mSecretTwelfthFlagHehe && !rs::isActiveDemo(koopa) &&
                !isNerve(player, &nrvPlayerActorHakoniwaHack) && !rs::isPlayer2D(player)) {
                Logger::log("Tried restarting bowser\n");
                al::updatePoseTrans(koopa, al::getTrans(player));
                al::resetQuatPosition(koopa, al::getQuat(al::getPlayerActor(koopa, 0)),
                                      al::getTrans(al::getPlayerActor(koopa, 0)));
                al::setGravity(koopa, -sead::Vector3f::ey);
                al::validateHitSensors(koopa);
                auto* head = al::getHitSensor(koopa, "SwoonHead");
                player->mHackCap->prepareLockOn(head);
                //                player->getPlayerHackKeeper()->requestForceHackStageStart(head, koopa->mCapTargetInfo,
                //                nullptr);
                //                player->getPlayerHackKeeper()->executeForceHackStageStart(player->mBodyHitSensor,
                //                player); player->mModelChanger->hideModel(); player->mModelChanger->syncHost(true);
                //                al::tryKillEmitterAndParticleAll(player);
                ////                *(CapTargetInfo**)OFFSET(player->mHackCap, 0x228) = koopa->mCapTargetInfo;
                //                player->mPlayerStateHack->startsHacked = true;
                //                al::setNerve(player, &nrvPlayerActorHakoniwaHack);
            } else
                Logger::log("Failed to restart bowser, %s %s %s %s\n", BTOC(flags->mSecretTwelfthFlagHehe),
                            BTOC(!rs::isActiveDemo(koopa)), typeid(*player->getNerveKeeper()->getCurrentNerve()).name(),
                            BTOC(!rs::isPlayer2D(player)));
        }

        if (!flags->mSecretTwelfthFlagHehe) {
            Orig(koopa);
            return;
        }

        _ZN2al9LiveActor8movementEv(koopa);
    }
};

struct KoopaControlWrapper : public exl::hook::impl::TrampolineHook<KoopaControlWrapper> {
    static void Callback(Koopa* koopa) {
        if (!rs::isActiveDemo(koopa)) {
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
    DemoCounter = false;
}

void koopaDrawDebug(sead::PrimitiveRenderer& renderer) {
    if (koopa == nullptr)
        return;
    for (int i = 0; i < koopa->getHitSensorKeeper()->getSensorNum(); i++) {
        auto* sensor = koopa->getHitSensorKeeper()->getSensor(i);
        renderer.drawSphere4x8(al::getSensorPos(sensor), al::getSensorRadius(sensor), sead::Color4f::cRed);
    }
}

// sead::Quatf* getQuatPtrKoopa(al::LiveActor* actor) { return al::getQuatPtr(actor); }

int blacklistedDemoTypes[] = {5, 3, 1};
const char* whitelistedDemoNames[] = {"シナリオカメラデモ", "プレイヤー含むデモ", "シャイン出現デモ", "ジャンゴデモ"};

struct DemoStartHandler : public exl::hook::impl::TrampolineHook<DemoStartHandler> {
    static bool Callback(al::DemoDirector* demoDirector, const char* name, int demoType) {
        Logger::log("Started demo (%d), wowie %s\n", demoType, name);

        int* foundType = std::find(std::begin(blacklistedDemoTypes), std::end(blacklistedDemoTypes), demoType);
        const char** foundName =
            std::find_if(std::begin(whitelistedDemoNames), std::end(whitelistedDemoNames),
                         [name](const char* whitelistedName) { return strcmp(whitelistedName, name) == 0; });
        Logger::log("%s found %s\n", BTOC(foundName != std::end(whitelistedDemoNames)),
                    BTOC(foundType == std::end(blacklistedDemoTypes)));

        if (koopa && (foundName != std::end(whitelistedDemoNames) || foundType == std::end(blacklistedDemoTypes))) {
            if (koopa->mPlayerHack)
                rs::sendMsgHackDemoStart(koopa->mHitSensorKeeper->getSensor(0), koopa->mHitSensorKeeper->getSensor(0));
            else {
                al::invalidateClipping(koopa);
                al::invalidateDepthShadowMap(koopa, "BodyHack");
                al::invalidateDepthShadowMap(koopa, "GroundHack");
                al::startAction(koopa, "Wait");
                al::startHitReaction(koopa, "死亡");
                al::updatePoseTrans(koopa, al::getTrans(al::getPlayerActor(koopa, 0)));
                al::resetQuatPosition(koopa, al::getQuat(al::getPlayerActor(koopa, 0)),
                                      al::getTrans(al::getPlayerActor(koopa, 0)));
                al::setGravity(koopa, -sead::Vector3f::ey);
                al::validateOcclusionQuery(koopa);
                al::setVelocityZero(koopa);
                al::invalidateHitSensors(koopa);
                setSecretTwelfthFlag(koopa, true);
                al::setNerve(koopa, &nrvKoopaWaitReset);
            }
        }
        bool res = Orig(demoDirector, name, demoType);
        return res;
    }
};

struct DemoEndHandler : public exl::hook::impl::TrampolineHook<DemoEndHandler> {
    static void Callback(al::DemoDirector* demoDirector, const char* name, int demoType) {
        Orig(demoDirector, name, demoType);
        Logger::log("Ended demo %s\n", name);
        //        DemoCounter = false;
    }
};

struct EndInitHandler : public exl::hook::impl::TrampolineHook<EndInitHandler> {
    static void Callback(al::Scene* scene) {
        Orig(scene);
        koopa = nullptr;
        demoage = 0;
        DemoCounter = 0;
    }
};

void onlyHackIfNeeded(Koopa* koopa, al::HitSensor* sensor, const CapTargetInfo* info, al::LiveActor* unused) {
    if (haveIBeenPwned)
        haveIBeenPwned = !rs::isPlayer2D(koopa);

    Logger::log("Hack setup start as hacked? %s\n", BTOC(haveIBeenPwned));
    if (haveIBeenPwned)
        rs::requestStageStartHack(koopa, sensor, info, unused);
    else {
        al::invalidateClipping(koopa);
        al::invalidateDepthShadowMap(koopa, "BodyHack");
        al::invalidateDepthShadowMap(koopa, "GroundHack");
        al::startAction(koopa, "Wait");
        al::startHitReaction(koopa, "死亡");
        al::updatePoseTrans(koopa, al::getTrans(al::getPlayerActor(koopa, 0)));
        al::resetQuatPosition(koopa, al::getQuat(al::getPlayerActor(koopa, 0)),
                              al::getTrans(al::getPlayerActor(koopa, 0)));
        al::setGravity(koopa, -sead::Vector3f::ey);
        al::validateOcclusionQuery(koopa);
        al::setVelocityZero(koopa);
        al::invalidateHitSensors(koopa);
        setSecretTwelfthFlag(koopa, true);
        al::setNerve(koopa, &nrvKoopaWaitReset);
        koopa->mKoopaPartialAnimator->clear();
    }
}

struct SendMsgStartHack : exl::hook::impl::TrampolineHook<SendMsgStartHack> {
    static bool Callback(HackCap* cap, al::HitSensor* bodySensor) {
        if (!isNerve(cap, &nrvHackCapLockOn))
            return false;
        Logger::log("send Msg hack Start!! %p %s %d %s\n", *OFFSET_CAST(cap, 0x1d0, al::HitSensor**),
                    BTOC(*(OFFSET_CAST(cap, 0x2a6, bool*))), cap->getNerveKeeper()->getCurrentStep(),
                    BTOC(*(OFFSET_CAST(cap, 0x2a6, bool*)) && !al::isGreaterStep(cap, 5)));
        if (*(OFFSET_CAST(cap, 0x2a6, bool*)) && !al::isGreaterStep(cap, 5)) {
            return rs::sendMsgStartHack(*OFFSET_CAST(cap, 0x1d0, al::HitSensor**), bodySensor);
        }
        return false;
    }
};

struct FixWalkInDemo : public exl::hook::impl::TrampolineHook<FixWalkInDemo> {
    static bool Callback(al::LiveActor* actor) {
        actor->kill();
        return false;
    }
};

/*
 * PlayerCameraFunction::setCameraHackName
 */

void koopaSideInit(patch::CodePatcher& patcher);
void enemyHooks(patch::CodePatcher& patcher);
void koopaPatchesInit() {
    patch::CodePatcher p(0x4ca0e4);
    p.BranchLinkInst((void*)&koopaInit); // setup init
    p.Seek(0x7de80);                     // todo: create global camera ticket
    p.WriteInst(inst::Nop());
    p.WriteInst(inst::Nop());
    p.Seek(0x4cfeb4); // disable checkpoint demo
    p.WriteInst(inst::Movz(reg::X0, 0x1));
    p.Seek(0x2e4410); // walk-in demo wait
    p.WriteInst(inst::Movz(reg::X0, 0x0));
    p.Seek(0x45ade4); // removes a isHackInvalidLifeRecovery check for PlayerJudgeRecoveryLifeFast (pretty sure this
                      // crashed something?)
    p.WriteInst(inst::MovRegister(reg::X0, reg::None64));
    p.Seek(0x3de4d4);
    p.WriteInst(inst::MovRegister(reg::X0, reg::None64));
    p.Seek(0x7e7c0);
    p.WriteInst(inst::Nop());
    p.Seek(0x4cb4b8); // warp hole
    p.WriteInst(inst::MovRegister(reg::X0, reg::None64));
    p.Seek(0x4cb4dc); // warp hole
    p.WriteInst(inst::Movz(reg::X0, 0x1));
    p.Seek(0x4b92fc); // resize Koopa creator memory to 0x230 from 0x220
    p.WriteInst(inst::Movz(reg::X0, 0x230));
    p.Seek(0x7ea70); // updateCollider for player push receiver
    p.BranchLinkInst((void*)&koopaUpdateCollider);
    p.Seek(0x7e6a0); // movement for switch to roll
    p.BranchLinkInst((void*)&koopaMovement);
    p.Seek(0x7da3c); // change max nerve state count to 3
    p.WriteInst(inst::Movz(reg::W2, 0x3));
    p.Seek(0x3f4200); // disable effect thingy for yukimaru state move
    p.WriteInst(inst::Nop());
    //    p.Seek(0x7e7dc);
    //    p.BranchLinkInst((void*)&is2DNerve);
    //    p.Seek(0x3f4b48);
    //    p.BranchLinkInst((void*)&getQuatPtrKoopa);
    p.Seek(0x3f4d08); //  for yukimaru state move
    p.WriteInst(inst::MovRegister(reg::W8, reg::None32));
    p.Seek(0x3f4b3c); ///  for yukimaru state move
    p.WriteInst(inst::Movz(reg::X0, 1));
    p.Seek(0x7fc14); // Koopa::exeMove
    p.WriteInst(inst::Movz(reg::W1, 16));
    //    p.Seek(0x4273f4); // Disable recovery
    //    p.WriteInst(inst::Ret());
    //    p.Seek(0x4202bc); // Disable if in demo
    //    p.WriteInst(inst::Movz(reg::X0, 0));
    //    p.Seek(0x4202f0); // we're in an active demo always
    //    p.WriteInst(inst::Movz(reg::X0, 0));
    //    p.Seek(0x420310); // isInRecovery: false
    //    p.WriteInst(inst::Movz(reg::X0, 0));
    p.Seek(0x420294); // PlayerActorHakoniwa::executePreMovementNerveChange
    p.WriteInst(inst::Movz(reg::X0, 0));
    p.Seek(0x7f280); // Disable life-up on start and demo start
    p.WriteInst(inst::Movz(reg::X0, 0));
    p.Seek(0x1f2998);
    p.WriteInst(inst::Movz(reg::X0, 1));
    p.WriteInst(inst::Ret());
    p.Seek(0x7dfdc);
    p.BranchLinkInst((void*)&onlyHackIfNeeded);
    koopaSideInit(p);
    enemyHooks(p);

    //    nvnImGui::addDrawFunc([]() {
    ////        Logger::log("Imgui");
    //        ImGui::Begin("PlayerHackKeeper", nullptr, ImGuiWindowFlags_NoCollapse);
    //        bool koopaExists = koopa, sceneExists = koopaExists && koopa->getSceneInfo(), playerExists = sceneExists
    //        &&  al::tryGetPlayerActor(koopa, 0); ImGui::BeginGroup(); ImGui::Checkbox("Koopa", &koopaExists);
    //        ImGui::Checkbox("SceneInfo", &sceneExists);
    //        ImGui::Checkbox("Player", &playerExists);
    //        ImGui::EndGroup();
    //        if (koopa && koopa->getSceneInfo() && al::tryGetPlayerActor(koopa, 0)) {
    //            auto* player = (PlayerActorHakoniwa*)al::tryGetPlayerActor(koopa, 0);
    //            auto* keeper = player->getPlayerHackKeeper();
    //            ImGui::Text("player %p", keeper->player);
    //            ImGui::Text("cap %p", keeper->cap);
    //            ImGui::Text("safetyPoint %p", keeper->safetyPoint);
    //            ImGui::Text("gap1 %p", keeper->gap1);
    //            ImGui::Text("gap2 %p", keeper->gap2);
    //            ImGui::Text("input %p", keeper->input);
    //            ImGui::Text("matrix %p", keeper->matrix);
    //            ImGui::Text("damageKeeper %p", keeper->damageKeeper);
    //            ImGui::Text("modelChanger %p", keeper->modelChanger);
    //            ImGui::Text("playerHeightChecker %p", keeper->playerHeightChecker);
    //            ImGui::Text("playerHitSensor %p", keeper->playerHitSensor);
    //            ImGui::BeginGroup();
    //            ImGui::Checkbox("Unused flag", &keeper->unused);
    //            ImGui::Checkbox("Hack cancelling ", &keeper->cancellingHack);
    //            ImGui::Checkbox("Hack demo", &keeper->hackDemoStarted);
    //            ImGui::Checkbox("Unused flag", &keeper->unusedFlag2);
    //            ImGui::Checkbox("Demo started", &keeper->regularDemoStarted);
    //            ImGui::Checkbox("Unused flag", &keeper->unusedFlag3);
    //            ImGui::Checkbox("Took damage", &keeper->tookDamage);
    //            ImGui::EndGroup();
    //            ImGui::Text("collisionFilter %p", keeper->collisionFilter);
    //            ImGui::Text("actor %p", keeper->actor);
    //            ImGui::Text("actorSensor %p", keeper->actorSensor);
    //            ImGui::Text("hackObjInfo %p", keeper->hackObjInfo);
    //            ImGui::Text("stageStartActorSensor %p", keeper->stageStartActorSensor);
    //            ImGui::Text("stageStartCapTargetInfo %p", keeper->stageStartCapTargetInfo);
    //            ImGui::Text("stageStartActor %p", keeper->stageStartActor);
    //            ImGui::Text("hackStartTexKeeper %p", keeper->hackStartTexKeeper);
    //            ImGui::Text("hackModel %p", keeper->hackModel);
    //        }
    //        ImGui::End();
    //    });

    KoopaReceiveMsg::InstallAtOffset(0x7ed3c);
    KoopaAttackSensor::InstallAtOffset(0x7ec00);
    FireballAttackSensor::InstallAtSymbol("_ZN13KoopaFireBall12attackSensorEPN2al9HitSensorES2_");
    KoopaControlWrapper::InstallAtSymbol("_ZN5Koopa7controlEv");
    KoopaMovementWrapper::InstallAtSymbol("_ZN5Koopa8movementEv");
    DemoStartHandler::InstallAtSymbol("_ZN2al12DemoDirector16requestStartDemoEPKc");
    DemoEndHandler::InstallAtSymbol("_ZN2al12DemoDirector14requestEndDemoEPKc");
    EndInitHandler::InstallAtSymbol("_ZN2al5Scene4killEv");
    SendMsgStartHack::InstallAtSymbol("_ZN7HackCap16sendMsgStartHackEPN2al9HitSensorE");
    FixWalkInDemo::InstallAtSymbol("_ZN2rs36requestStartDemoSceneStartPlayerWalkEPN2al9LiveActorE");
}
