#include "ForwardDecls.h"
#include <al/Library/LiveActor/ActorSensorFunction.h>
#include <al/Library/LiveActor/LiveActor.h>
#include <lib.hpp>
#include <logger/Params.h>

struct ExplodeIntoPiecesBigger : exl::hook::impl::ReplaceHook<ExplodeIntoPiecesBigger> {
    static void Callback(al::LiveActor* actor) {
        const s32 length = par::get("Length", 60);
        const f32 regularSize = par::get("RegularSize", 1000.0f);
        const f32 wideSize = par::get("WideSize", 1000.0f);
        const f32 scale = par::get("Scale", 10.0f);

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
        }
    }
};

void bulletPatches() {
    ExplodeIntoPiecesBigger::InstallAtSymbol("_ZN10TankBullet10exeExplodeEv");
}
