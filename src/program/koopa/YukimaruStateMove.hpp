#pragma once

#include <al/Library/Nerve/NerveStateBase.h>

class YukimaruInput;

class YukimaruStateMove : al::ActorStateBase {
    char _data[0xb8 - sizeof(al::ActorStateBase)];
public:
    YukimaruStateMove(al::LiveActor* actor, YukimaruInput* input, sead::Quatf* rotationQuat);
    void attackSensor(al::HitSensor* target, al::HitSensor* source);
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* source, al::HitSensor* target);
    void updateMove();
};
