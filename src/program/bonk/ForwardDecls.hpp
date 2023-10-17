#pragma once

#include <agl/TextureData.h>
#include <agl/gpu.h>
#include <agl/util.h>
#include <al/Library/Nerve/NerveSetupUtil.h>
#include <al/Library/LiveActor/LiveActor.h>


namespace al {
    f32 getRandom();
    f32 getRandom(f32);
    f32 getRandom(f32, f32);
    s32 getRandom(s32);
    s32 getRandom(s32, s32);

    class OneMeshFixMapParts {};
    class LiveActor;

    void startHitReaction(const al::LiveActor*, const char*);
    void startAction(al::LiveActor*, const char*);
    void emitRadialBlur(const al::LiveActor*, const sead::Vector3f&, float, float, float, float,
                        float, float, int, int, bool);
} // namespace al
namespace agl {
    class TextureDataInitializerRAW {
    public:
        static void initialize(agl::TextureData*, agl::GPUMemVoidAddr, unsigned long, agl::TextureFormat, int, int,
                               sead::Heap*);
    };
} // namespace agl
namespace {
    MAKE_NERVE_FAKE(StageScene, Play)
    MAKE_NERVE_FAKE(TankBullet, Move)
    MAKE_NERVE_FAKE(TankBullet, Start)
    MAKE_NERVE_FAKE(TankBullet, Explode)
}

class FixMapPartsCapHanger {};
class FixMapPartsForceSafetyPoint {};
class FixMapPartsFukankunZoomCapMessage {};

struct PlayerOxygen {
public:
    bool isTriggerDamage() const;
};

class PlayerTrigger {
public:
    enum class EActionTrigger {
        IceWaterDamage = 4,
        OxygenDamage = 7,
    };
    void set(EActionTrigger trigger);
};

class PlayerDamageKeeper {
public:
    void damage(s32 invalidFrames);
};

class TankBullet : public al::LiveActor {
public:
    TankBullet(const char* name);
    void init(const al::ActorInitInfo&) override;
    void attackSensor(al::HitSensor*, al::HitSensor*) override;
    bool receiveMsg(const al::SensorMsg*, al::HitSensor*, al::HitSensor*) override;
    void shoot(const sead::Vector3<float>&, const sead::Vector3<float>&, int, bool, bool);
    void shootByPlayer(const sead::Vector3<float>& trans, const sead::Vector3<float>& vel,
                       const sead::Vector3<float>& dir, const sead::Vector3<float>&, float, int);

    void exeMove();
    void exeStart();
    void exeMovePlayer();
    void exeExplode();
    void exeYoshiEat();

    int maxStepAlive;
    bool shotByPlayer;
    sead::Vector3f vecA;
    sead::Vector3f vecB;
    sead::Vector3f vecC;
    float aFloat;
    bool smallExplode;
};