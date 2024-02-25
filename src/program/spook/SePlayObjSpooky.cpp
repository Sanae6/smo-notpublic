#include "SePlayObjSpooky.hpp"
#include "al/Library/LiveActor/ActorClippingFunction.h"
#include "al/Library/LiveActor/ActorInitInfo.h"
#include "al/Library/LiveActor/ActorMovementFunction.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "logger/Params.h"
#include "utils/ForwardDecls.hpp"

namespace sp {
  void SePlayObjSpooky::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, "SePlayObjSpooky", nullptr);
    al::invalidateClipping(this);
  }

  void SePlayObjSpooky::playSound(const char *sound) {
    float lower = par::get("DistLowerBound", 800.0f);
    float upper = par::get("DistUpperBound", 1000.0f);
    float x = al::getRandom(lower, upper);
    float z = al::getRandom(lower, upper);
    auto trans = al::getTrans(player) + sead::Vector3f(x, 0, z - (upper / 2.0f));
    al::setTrans(this, trans);
    float dist = al::calcDistance(this, player);
    Logger::log("Played sound at distance: %f\n", dist);
    al::startSe(this, sound);
  }
} // namespace sp
