#pragma once

#include <al/Library/LiveActor/LiveActor.h>
#include <al/Library/Scene/Scene.h>
#include "game/Interfaces/IUseDimension.h"
#include "game/Interfaces/IUsePlayerHack.h"
#include "game/Player/HackCap/CapTargetInfo.h"
#include "koopa/PlayerPushReceiver.hpp"

struct KoopaPartialAnimator {
    void appear();
    void clear();
    void attackSensor(al::HitSensor* target, al::HitSensor* source);
    void stop();
};

class KoopaRoll;

class Koopa : public al::LiveActor, public IUseDimension {
public:
    Koopa(const char*);
    void init(const al::ActorInitInfo& initInfo) override;
    void appear() override;
    void kill() override;
    void movement() override;
    void updateCollider() override;

    void exeBind();
    void exeBindHackStart();
    void exeDead();
    void exeDeadFall();
    void exeFall();
    void exeJump();
    void exeStart();
    void exeSwoon();
    void exeSwoonReaction();
    void exeWait();
    void exe2D();
    void exeRoll();

    ActorDimensionKeeper * getActorDimensionKeeper() const override;

    IUsePlayerHack* mPlayerHack;
    CapTargetInfo* mCapTargetInfo;
    void* mKoopa2D;
    void* mKoopaPuppet;
    ActorDimensionKeeper* mActorDim;
    KoopaPartialAnimator* mKoopaPartialAnimator;
    void* mKoopaHackStopCtrl;
    void* mUnk1;
    sead::Vector3f* mUnk2;
    char mPadding[0xC2];
    PlayerPushReceiver* playerPushReceiver;
    al::Scene* scene;
};
