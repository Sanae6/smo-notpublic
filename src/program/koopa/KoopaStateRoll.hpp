//#pragma once
//
//#include "Koopa.hpp"
//#include "YukimaruInput.hpp"
//#include "YukimaruStateMove.hpp"
////#include "al/nerve/ActorStateBase.h"
//
//class KoopaStateRoll : public al::ActorStateBase, public YukimaruInput {
//public:
//    KoopaStateRoll(const char* name, Koopa* koopa);
//    void appear() override;
//    void kill() override;
//    void control() override;
//    void attackSensor(al::HitSensor* target, al::HitSensor* source);
//    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* source, al::HitSensor* target);
//    bool isTriggerJump() const override;
//    bool isHoldJump() const override;
//    bool isSwingDirLeft() const override;
//    bool isSwingDirRight() const override;
//    void calcInputVec(sead::Vector3f&) override;
//
//
//private:
//    Koopa* getKoopa() const { return (Koopa*)this->mLiveActor; }
//    YukimaruStateMove moveState;
//    sead::Quatf* rotQuat;
//};
