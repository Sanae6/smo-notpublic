//#include "KoopaStateRoll.hpp"
//#include "KoopaRoll.hpp"
//#include "al/util/LiveActorUtil.h"
//#include <al/util.hpp>
//#include <logger/Logger.hpp>
//
//namespace rs {
//    void hideHackCap(IUsePlayerHack*);
//    void showHackCap(IUsePlayerHack*);
//    void calcHackerMoveVec(sead::Vector3f*, const IUsePlayerHack *, const sead::Vector3f &);
//} // namespace rs
//
//KoopaStateRoll::KoopaStateRoll(const char* name, Koopa* koopa)
//    : ActorStateBase(name, koopa), moveState(koopa->koopaRoll, this, koopa->koopaRoll->getQuatPtr()), rotQuat(koopa->koopaRoll->getQuatPtr()) {}
//
//void KoopaStateRoll::control() {
//    moveState.updateMove();
//    al::copyPose(getKoopa(), getKoopa()->koopaRoll);
//    al::hideModelIfShow(getKoopa());
//    if (al::isDead(getKoopa()->koopaRoll)) kill();
//}
//void KoopaStateRoll::appear() {
//    al::setFront(getKoopa()->koopaRoll, al::getFront(getKoopa()));
//    al::setTrans(getKoopa()->koopaRoll, al::getTrans(getKoopa()));
//    al::offCollide(getKoopa());
//    al::setVelocityZero(getKoopa());
//    al::stopAction(getKoopa());
//    al::hideSilhouetteModelIfShow(getKoopa());
//    rs::hideHackCap(getKoopa()->mPlayerHack);
//    getKoopa()->mKoopaPartialAnimator->stop();
//    al::startAction(getKoopa(), "Wait");
//
//    sead::Quatf quat;
//    al::calcQuat(&quat, getKoopa());
//    *rotQuat = quat;
//    getKoopa()->koopaRoll->appear();
//    al::hideModelIfShow(getKoopa());
//    NerveStateBase::appear();
//}
//
//void KoopaStateRoll::kill() {
//    getKoopa()->koopaRoll->kill();
//    al::onCollide(getKoopa());
//    al::showModelIfHide(getKoopa());
//    rs::showHackCap(getKoopa()->mPlayerHack);
//    al::showSilhouetteModelIfHide(getKoopa());
//    NerveStateBase::kill();
//}
//void KoopaStateRoll::attackSensor(al::HitSensor* target, al::HitSensor* source) {
//    moveState.attackSensor(target, source);
//}
//bool KoopaStateRoll::receiveMsg(const al::SensorMsg* msg, al::HitSensor* source, al::HitSensor* target) {
//    return moveState.receiveMsg(msg, source, target);
//}
//bool KoopaStateRoll::isTriggerJump() const { return false; }
//bool KoopaStateRoll::isHoldJump() const { return false; }
//bool KoopaStateRoll::isSwingDirLeft() const { return false; }
//bool KoopaStateRoll::isSwingDirRight() const { return false; }
//void KoopaStateRoll::calcInputVec(sead::Vector3f& vec) {
//    rs::calcHackerMoveVec(&vec, getKoopa()->mPlayerHack, -al::getGravity(getKoopa()->koopaRoll));
////    getKoopa()->mPlayerHack->getPlayerHackKeeper()->getPlayerInput()->calcMoveInput(
////        &vec, al::getGravity(getKoopa()->koopaRoll));
//    Logger::log("pussy %.2f %.2f %.2f\n", vec.x, vec.y, vec.z);
//}
