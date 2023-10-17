#include <al/Library/LiveActor/ActorMovementFunction.h>
#include <bonk/mods/GunMod.hpp>

namespace bm {
    struct GunAttackSensor : Trampoline<GunAttackSensor> {
        static void Callback(TankBullet* bullet, al::HitSensor* target, al::HitSensor* source) {
            auto* sourceHost = al::getSensorHost(source);
            // Logger::log("HIT SOMETHING source=%s, target=%s, sourcehost=%s, sourcename=%s\n",
            // source->mName,
            //             target->mName, typeid(*sourceHost).name(), sourceHost->getName());
            if (al::isEqualString(sourceHost->getName(), "TankBullet"))
                return;

            if (bullet->shotByPlayer)
                switch (handleAddonSensorContent(bullet, target, source)) {
                case AttackResult::Ignore:
                    return;
                case AttackResult::Continue:
                    break;
                case AttackResult::Explode:
                    goto hitReactionExplode;
                }

            if (al::isNerve(bullet, &nrvTankBulletStart) || al::isNerve(bullet, &nrvTankBulletYoshiEat) ||
                (bullet->shotByPlayer && !al::isSensorEye(target) && al::isSensorMapObj(source) &&
                 rs::sendMsgWeaponItemGet(source, target))) {
                return;
            }

            if (al::isNerve(bullet, &nrvTankBulletExplode)) {
                // Logger::log("HIT IN EXPLODE source=%s, target=%s, sourcehost=%s, sourcename=%s\n",
                // source->mName, target->mName,
                //         typeid(*sourceHost).name(), sourceHost->getName());

                if (!al::isSensorEye(target)) {
                    // Logger::log("So fucking sensing\n");
                    bullet->kill();
                    return;
                }

                if (al::isSensorName(target, "Explosion")) {
                    if (bullet->smallExplode && al::isSensorPlayer(source))
                        return;
                    al::sendMsgExplosion(source, target, nullptr);
                    return;
                }
                if (!bullet->shotByPlayer) {
                    return;
                }
                if (!al::isSensorName(target, "ExplosionWide")) {
                    return;
                }
                rs::sendMsgTankExplosion(source, target);
                return;
            }

            if (al::isSensorPlayer(source))
                return;

            if (!bullet->shotByPlayer) {
                if (!al::sendMsgExplosion(source, target, NULL))
                    return;
                goto hitReactionExplode;
            }

            if (rs::sendMsgTankBulletNoReaction(source, target)) {
                bullet->kill();
                return;
            }

            if (!rs::sendMsgTankBullet(source, target) && !al::sendMsgExplosion(source, target, NULL))
                return;

            {
                const char* balls = al::isExistCollisionParts(al::getSensorHost(source)) &&
                                            al::isValidCollisionParts(al::getSensorHost(source)) ?
                                        "キャプチャショット壁あたり" :
                                        "キャプチャショットオブジェあたり";
                al::startHitReaction(bullet, balls);
            }
        hitReactionExplode:
            al::startAction(bullet, "Explode");
            al::startHitReaction(bullet, "爆発");
            al::setVelocityZero(bullet);
            al::validateHitSensor(bullet, "Explosion");
            al::validateHitSensor(bullet, "ExplosionWide");
            al::setNerve(bullet, &nrvTankBulletExplode);
        }
    };
    void GunMod::activate() { Mod::activate(); }
    void GunMod::sceneStart(const al::ActorInitInfo& initInfo) { Mod::sceneStart(initInfo);

    }

} // namespace bm
