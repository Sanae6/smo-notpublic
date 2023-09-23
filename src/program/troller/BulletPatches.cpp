#include "ForwardDecls.h"
#include <al/Library/LiveActor/ActorSensorFunction.h>
#include <al/Library/LiveActor/LiveActor.h>
#include <lib.hpp>

struct ExplodeIntoPiecesBigger : exl::hook::impl::ReplaceHook<ExplodeIntoPiecesBigger> {
    static void Callback(al::LiveActor* actor) {
        const s32 length = 60;
        const f32 regularSize = 1000.0f;
        const f32 wideSize = 1000.0f;
        const f32 scale = 10;

        sead::Vector3 scaleVec(scale, scale, scale);
        al::setEffectEmitterVolumeScale(actor,"Explosion", scaleVec);
        al::setEffectParticleScale(actor,"Explosion", scaleVec);
        al::setSensorRadius(actor, "Explosion", al::calcNerveRate(actor, length / 2) * regularSize);
        al::setSensorRadius(actor, "ExplosionWide", al::calcNerveRate(actor, length / 2) * wideSize);
        if (al::isGreaterEqualStep(actor, length)) {
            al::tryAddRippleLarge(actor);
            al::setSensorRadius(actor, "Explosion", 0.0);
            al::setSensorRadius(actor, "ExplosionWide", 0.0);
            al::invalidateHitSensor(actor, "Explosion");
            al::invalidateHitSensor(actor, "ExplosionWide");
            actor->kill();
            return;
        }
        return;
    }
};

void bulletPatches() {
    ExplodeIntoPiecesBigger::InstallAtSymbol("_ZN10TankBullet10exeExplodeEv");
}
