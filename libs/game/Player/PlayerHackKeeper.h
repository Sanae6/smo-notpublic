/**
 * @file PlayerHackKeeper.h
 * @brief Contains info on the current hack (capture)
 */

#pragma once

#include "game/Interfaces/IUsePlayerHack.h"
#include "game/Player/HackCap.h"
#include "game/Player/HackCap/CapTargetInfo.h"
#include "game/Player/PlayerCollider.h"
#include "game/Player/PlayerInput.h"
#include <al/Library/LiveActor/LiveActor.h>

struct HackEndParam;
struct PlayerRecoverySafetyPoint;
struct PlayerDamageKeeper;
struct IPlayerModelChanger;
struct IUsePlayerHeightCheck;

namespace al {
    struct CollisionPartsFilterBase;
}

class PlayerHackKeeper {
public:
    PlayerHackKeeper(al::LiveActor*, HackCap*, PlayerRecoverySafetyPoint*, PlayerInput const*, sead::Matrix34f const*,
                     PlayerDamageKeeper const*, IPlayerModelChanger const*, IUsePlayerHeightCheck const*);
    void createHackModel(al::ActorInitInfo const&);
    void startHack(al::HitSensor*, al::HitSensor*, al::LiveActor*);
    void setupHack(al::HitSensor*, al::HitSensor*, al::LiveActor*);
    void endHack(HackEndParam const*);
    void endHackStartDemo(al::LiveActor*);
    void startHackStartDemo(al::LiveActor*);
    void startHackStartDemoPuppetable(al::LiveActor*);
    void addHackStartDemo(al::LiveActor*);
    void appearHackDemoModel(sead::Matrix34f const&, float);
    void updateHackDemoModel(sead::Matrix34f const&, float);
    void deleteHackDemoModelEffect(void);
    void killHackDemoModel(void);
    bool isActiveHackStartDemo(void) const;
    void recordHack(void);
    void cancelHackArea(void);
    void cancelHack(void);
    void cancelForceRecovery(void);
    void tryEscapeHack(void);
    void sendTransferHack(void);
    void sendMarioDemo(void);
    void forceKillHack(void);
    void sendMarioDead(void);
    void sendMarioInWater(void);
    void sendMarioDeathArea(void);
    void sendMsgEnableMapCheckPointWarp(void);
    void sendMsgSelfCeilingCheckMiss(void);
    void receiveRequestTransferHack(al::HitSensor*);
    void requestDamage(void);
    void receiveRequestDamage(void);
    void sendSyncDamageVisibility(void);
    void pushWorldEndBorder(sead::Vector3f const&);
    const char* getCurrentHackName(void) const;
    PlayerCollider* getPlayerCollision(void);
    float getHackGuideHeight(void);
    bool isHackGuideEnable(void) const;
    float getHackStayGravityMargine(void);
    void* getCollisionPartsFilter(void);
    bool isHackGroupTalkScare(void) const;
    bool isHackNoCollisionMsg(void) const;
    bool isHackNoSeparateCameraInput(void) const;
    bool isHackUsePlayerCollision(void) const;
    bool isHackCancelCeilingCheck(void) const;
    bool isHackInvalidLifeRecovery(void) const;
    void requestForceHackStageStart(al::HitSensor*, CapTargetInfo const*, al::LiveActor*);
    void executeForceHackStageStart(al::HitSensor*, IUsePlayerHack*);
    void startDemo(void);
    void endDemo(void);

    PlayerInput* getPlayerInput() { return input; }
    al::LiveActor* player;
    HackCap* cap;
    PlayerRecoverySafetyPoint* safetyPoint;
    void* gap1;
    void* gap2;
    PlayerInput* input;
    sead::Matrix34f* matrix;
    PlayerDamageKeeper* damageKeeper;
    IPlayerModelChanger* modelChanger;
    IUsePlayerHeightCheck* playerHeightChecker;
    al::HitSensor* playerHitSensor;
    bool unused;
    bool cancellingHack;
    bool hackDemoStarted;
    bool unusedFlag2;
    bool regularDemoStarted;
    bool unusedFlag3;
    bool tookDamage;
    al::CollisionPartsFilterBase* collisionFilter;
    al::LiveActor* actor;
    al::HitSensor* actorSensor;
    struct HackObjInfo* hackObjInfo;
    al::HitSensor* stageStartActorSensor;
    CapTargetInfo* stageStartCapTargetInfo;
    al::LiveActor* stageStartActor;
    struct PlayerHackStartTexKeeper* hackStartTexKeeper;
    al::LiveActor* hackModel;
//    sead::PtrArray<int> hackModelArray1;
//    sead::Matrix34f* hackModelArray2;
//    int unknownA, unknownB, unknownC;

};
