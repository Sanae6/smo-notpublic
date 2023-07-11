//#include "koopa/KoopaRoll.hpp"
//#include "al/util.hpp"
//#include "al/util/LiveActorUtil.h"
//#include "al/util/NerveUtil.h"
//#include <al/util/InputUtil.h>
//
//namespace {
//    NERVE_DEF(KoopaRoll, Appear);
//    NERVE_DEF(KoopaRoll, Move);
//    NERVE_DEF(KoopaRoll, Unroll);
//}
//
//KoopaRoll::KoopaRoll(const char* name) : al::LiveActor(name), mRotQuat() {}
//
//void KoopaRoll::init(const al::ActorInitInfo& initInfo) {
//    al::initActorWithArchiveName(this, initInfo, "KoopaDamageBallBomb", nullptr);
//    al::initJointControllerKeeper(this, 1);
//    al::initJointGlobalQuatController(this, &mRotQuat, "Rotate");
//    al::initNerve(this, &nrvKoopaRollMove, 0);
//    al::setScaleAll(this, 0.25f);
//
//    this->kill();
//}
//
//bool KoopaRoll::isMoving() const {
//    return al::isNerve(this, &nrvKoopaRollMove);
//}
//
//void KoopaRoll::appear() {
//    LiveActor::appear();
//    al::setNerve(this, &nrvKoopaRollMove);
//}
//void KoopaRoll::exeAppear() {
//    al::scaleVelocity(this, 0.3);
//}
//void KoopaRoll::exeMove() {
//    if (al::isFirstStep(this)) {
//        al::setVelocity(this, al::getFront(this) * 50.0f);
//    }
////    al::updatePoseQuat(this, mRotQuat);
//    al::rotateQuatRollBall(&mRotQuat, mRotQuat, al::getVelocity(this), -al::getGravity(this), al::getColliderRadius(this));
//    if (al::isPadReleaseZL(-1) || !al::isPadHoldZL(-1)) {
//        al::setNerve(this, &nrvKoopaRollUnroll);
//    }
//}
//void KoopaRoll::exeUnroll() {
//    al::setVelocityZero(this);
//    kill();
//}
