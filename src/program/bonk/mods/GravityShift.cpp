#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/Math/MathLerpUtil.h>
#include <bonk/mods/GravityShift.hpp>

namespace bm {
    namespace {
        MAKE_NERVE_BOTH(GravityShift, Pick);
        MAKE_NERVE_BOTH(GravityShift, Lerp);
        MAKE_NERVE_BOTH(GravityShift, End);
    } // namespace
    GravityShift::GravityShift() { initNerve(&GravityShiftNrvPick::sInstance, 0); }
    void GravityShift::exePick() {
        start = al::getGravity(getMario());
        end.x = al::getRandom(-1.0f, 1.0f) * par::get("GravityConeRange", 1.0f);
        end.z = al::getRandom(-1.0f, 1.0f) * par::get("GravityConeRange", 1.0f);
        end.y = -2.0f;
        end.normalize();
        end *= par::get("GravityLevel", 1.0f);
        al::setNerve(this, &GravityShiftNrvLerp::sInstance);
    }
    void GravityShift::exeLerp() {
        int time = par::get("GravityLerpFrames", 1200);
        al::lerpVec(al::getGravityPtr(getMario()), start, end, al::calcNerveRate(this, time));
        if (al::isGreaterEqualStep(this, time))
            al::setNerve(this, &GravityShiftNrvEnd::sInstance);
    }
    void GravityShift::exeEnd() {
        pauseForSeconds(par::get("GravityCooldownSecs", 30));
        al::setNerve(this, &GravityShiftNrvPick::sInstance);
    }
    void GravityShift::deactivate() {
        Mod::deactivate();
        if (inScene())
            al::setGravity(getMario(), -sead::Vector3f::ey);
    }
    void GravityShift::sceneStart(const al::ActorInitInfo& initInfo) {
        Mod::sceneStart(initInfo);
        start = al::getGravity(getMario());
        end = al::getGravity(getMario());
        if (active)
            al::setNerve(this, &GravityShiftNrvEnd::sInstance);
    }
} // namespace bm
