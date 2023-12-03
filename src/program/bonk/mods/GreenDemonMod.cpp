#include <al/Library/LiveActor/ActorActionFunction.h>
#include <al/Library/LiveActor/ActorFlagFunction.h>
#include <al/Library/LiveActor/ActorMovementFunction.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/Math/MathVectorUtil.h>
#include <bonk/mods/GreenDemonMod.hpp>
#include <game/Player/PlayerFunction.h>
#include <helpers/PlayerHelper.h>
#include <logger/Logger.hpp>
#include <logger/Params.h>

namespace bm {
    namespace {
        MAKE_NERVE_BASE(GreenDemon, PopUp);
        MAKE_NERVE_BASE(GreenDemon, Land);
        MAKE_NERVE_BASE(GreenDemon, Move);
    } // namespace
    struct GreenDemon : public al::LiveActor {
        PlayerActorHakoniwa* mario;
        sead::Vector3f lastDirection;
        bool lerpDisabled = false;
        int lerpDisableTimer = 0;
        GreenDemon(PlayerActorHakoniwa* mario) : al::LiveActor("GreenDemon"), mario(mario) {}

        void init(const al::ActorInitInfo& info) override {
            al::initActorWithArchiveName(this, info, "VolleyballBall", nullptr);
            al::initNerve(this, &GreenDemonNrvPopUp::sInstance, 0);
            al::offCollide(this);
            al::invalidateClipping(this);

            kill();
        }

        void appear() override {
            al::LiveActor::appear();
            al::setTrans(this, al::getTrans(mario) + sead::Vector3f::ey * par::get("DemonSpawnY", 160.0f));
            pause();
        }

        void pause() { al::setNerve(this, &GreenDemonNrvPopUp::sInstance); }
        void exePopUp() {
            if (al::isFirstStep(this))
                al::startAction(this, "Reaction");

            if (al::isActionEnd(this))
                al::setNerve(this, &GreenDemonNrvLand::sInstance);
        }
        void exeLand() {
            if (al::isFirstStep(this)) {
                al::startAction(this, "Smash");
                al::startHitReaction(this, "スマッシュ");

                al::emitRadialBlur(this, al::getTrans(this), par::get("DemonLowerRad", 50000.0f),
                                   par::get("DemonHigherRad", 100000.0), 0.2, 0.2, 1.0, 0.0,
                                   par::get("DemonDuration", 128), -1, false);
            }
            al::setActionFrameRate(this, al::calcNerveEaseInOutRate(this, par::get("DemonLandEndFrame", 120)) / 2.0f);

            if (al::isGreaterEqualStep(this, par::get("DemonLandEndFrame", 120)))
                al::setNerve(this, &GreenDemonNrvMove::sInstance);
        }
        void exeMove() {
            auto rate = al::calcNerveEaseInOutRate(this, par::get("DemonLandEndFrame", 120));
            al::setActionFrameRate(this, rate / 2.0f + 0.5f);

            al::setTrans(this, al::getTrans(this) + (lastDirection * rate * par::get("DemonSpeed", 4.0f)));
        }

        void control() override {
            if (PlayerFunction::isPlayerDeadStatus(this))
                return;
            if (al::calcDistance(this, mario) < par::get("DemonNoLerpRange", 1000.0f)) {
                if (lerpDisableTimer < 0)
                    lerpDisabled = true;
                else
                    lerpDisableTimer--;
            } else {
                lerpDisabled = false;
                lerpDisableTimer = 600;
            }

            sead::Vector3f marioPos = al::getTrans(mario) + sead::Vector3f(0, 100, 0);
            sead::Vector3f dir = marioPos - al::getTrans(this);

            if (lerpDisabled)
                lastDirection = dir;
            else
                al::lerpVec(&lastDirection, dir, lastDirection, par::get("DemonSlerp", 1.0f));

            sead::Vector3f up;
            sead::Vector3CalcCommon<float>::cross(up, al::getTrans(this), marioPos);

            if (lastDirection.length() > 1)
                lastDirection.normalize();
            up.normalize();
            sead::Quatf dirQuat;
            al::makeQuatUpFront(&dirQuat, up, lastDirection);

            al::updatePoseQuat(this, dirQuat);
        }

        bool receiveMsg(const al::SensorMsg* message, al::HitSensor* source, al::HitSensor* target) override {
            if (!al::isMsgItemGetAll(message) || !al::isNerve(this, &GreenDemonNrvMove::sInstance))
                return false;
            if (!par::get("DemonDisableKills", false)) {
                al::sendMsgEnemyAttack(source, target);
            }
            //                PlayerHelper::killPlayer(this);
            return true;
        }
    };
    namespace {
        MAKE_NERVE_IMPL(GreenDemon, PopUp);
        MAKE_NERVE_IMPL(GreenDemon, Land);
        MAKE_NERVE_IMPL(GreenDemon, Move);
    } // namespace
    void GreenDemonMod::sceneStart(const al::ActorInitInfo& initInfo) {
        Mod::sceneStart(initInfo);
        oneUp = alloc<GreenDemon>(getMario());
        oneUp->init(initInfo);
    }

    void GreenDemonMod::marioGainsMovement() { oneUp->appear(); }
    void GreenDemonMod::activate() {
        Mod::activate();
        if (inScene())
            oneUp->appear();
    }
    void GreenDemonMod::marioBonked() { oneUp->pause(); }
    void GreenDemonMod::deactivate() {
        Mod::deactivate();
        if (inScene())
            oneUp->kill();
    }

} // namespace bm
