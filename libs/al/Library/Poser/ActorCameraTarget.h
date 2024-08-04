#pragma once

#include <math/seadVector.h>
#include <al/Library/LiveActor/LiveActor.h>
#include "al/Library/Camera/CameraTargetBase.h"

namespace al
{
    class ActorCameraTarget : public al::CameraTargetBase
    {
    public:
        ActorCameraTarget(al::LiveActor const *, float, sead::Vector3f const *);

        const char * getTargetName() override;
        bool isCollideGround() const override;
        bool isInWater() const override;

        void calcTrans(sead::Vector3f *) override;
        void calcSide(sead::Vector3f *) const override;
        void calcUp(sead::Vector3f *) const override;
        void calcFront(sead::Vector3f *) const override;
        void calcGravity(sead::Vector3f *) const override;
        void calcVelocity(sead::Vector3f *) const override;

        al::LiveActor *actor; // 0x10
        sead::Vector3f *pos; // 0x18
        float distance; // 0x20
    };
};