#pragma once

#include <al/Library/Area/AreaObj.h>
#include <al/Library/Area/IUseAreaObj.h>
#include <al/Library/LiveActor/LiveActor.h>
#include <al/Library/Nerve/NerveSetupUtil.h>
#include <game/Interfaces/IUsePlayerCollision.h>
#include <game/Interfaces/IUsePlayerHack.h>

namespace al {
    struct AreaObjGroup {
        const char* name;
        al::AreaObj* areas;
        s32 count;
        s32 capacity;

        al::AreaObj* getAreaObj(s32 index) const;
    };
    al::AreaObjGroup* tryFindAreaObjGroup(const al::IUseAreaObj* areaObjUser, const char* areaName);
    class WaterSurfaceFinder;
    class AreaObjFactory;

    void startHitReaction(const al::LiveActor* actor, const char* hitReactionName);
    void tryDeleteEffect(al::IUseEffectKeeper* effectUser, const char* effectName);
    bool isExistPrePassLight(const LiveActor* actor, const char* lightName);
    bool isActivePrePassLight(const LiveActor* actor, const char* lightName);
    bool killPrePassLight(const LiveActor* actor, const char* lightName, s32 fadeMaybe);
    void tryAddRippleLarge(const al::LiveActor* actor);
    void setEffectEmitterVolumeScale(al::IUseEffectKeeper* effectUser, const char* effectName, const sead::Vector3f& scale);
    void setEffectParticleScale(al::IUseEffectKeeper* effectUser, const char* effectName, const sead::Vector3f& scale);
} // namespace al

namespace rs {
    bool isActiveDemo(const al::LiveActor* actor);
    bool isPlayer2D(const al::LiveActor* actor);
    IUsePlayerHack* startHack(al::HitSensor*, al::HitSensor*, al::LiveActor* actor = nullptr);
    bool isOnGround(const al::LiveActor* actor, const IUsePlayerCollision* collision);
} // namespace rs

namespace {
    MAKE_NERVE_FAKE(PlayerActorHakoniwa, Hack)
    MAKE_NERVE_FAKE(PlayerActorHakoniwa, Abyss)
    MAKE_NERVE_FAKE(Pukupuku, BlowDownFromCapture)
} // namespace
