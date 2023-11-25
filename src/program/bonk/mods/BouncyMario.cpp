#include <al/Library/LiveActor/ActorMovementFunction.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <bonk/mods/BouncyMario.hpp>

namespace bm {
    void BouncyMario::control() {
        Mod::control();
        auto vel = al::getVelocity(getMario());
        vel.normalize();
        auto scale = abs(vel * par::get("BouncyModifier", 1.0f))
                     + (sead::Vector3f::ones);
        al::setScale(getMario(), scale);
    }
} // namespace bm
