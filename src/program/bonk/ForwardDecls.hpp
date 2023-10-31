#pragma once

#include <agl/TextureData.h>
#include <agl/gpu.h>
#include <agl/util.h>
#include <al/Library/Camera/CameraTargetBase.h>
#include <al/Library/Layout/LayoutActor.h>
#include <al/Library/LiveActor/LiveActor.h>
#include <al/Library/Nerve/NerveSetupUtil.h>
#include <gfx/gfx_DescriptorSlot.h>
#include <gfx/gfx_Device.h>

namespace eui {
    bool RegisterSlotForTexture(nn::gfx::DescriptorSlot*, nn::gfx::TTextureView<nn::gfx::ApiVariationNvn8> const&,
                                void*);
}
namespace nn {
    namespace gfx {
        using Device = nn::gfx::TDevice<nn::gfx::ApiVariationNvn8>;
    }
    namespace font {
        using RegisterTextureViewSlot = bool(nn::gfx::DescriptorSlot* pDstSlot, const nn::gfx::TextureView& textureView,
                                             void* pUserData);
        class ResFontBase {
        public:
            void RegisterTextureViewToDescriptorPool(RegisterTextureViewSlot slotPtr, void* userData);
        };
        class ResFont : public ResFontBase {
        public:
            ResFont();
            bool SetResource(gfx::Device* device, void* fontData,
                             gfx::TMemoryPool<gfx::ApiVariationNvn8>*, ptrdiff_t memoryPoolOffset,
                             size_t memoryPoolSize);
        };
        struct BinaryFileHeader {
            u32 signature;
            u16 byteOrder;
            u16 headerSize;
            u32 version;
            u32 fileSize;
            u16 dataBlocks;
            u16 reserved;
        };
    } // namespace font
} // namespace nn
namespace al {
    f32 getRandom();
    f32 getRandom(f32);
    f32 getRandom(f32, f32);
    s32 getRandom(s32);
    s32 getRandom(s32, s32);

    class OneMeshFixMapParts {};
    class LiveActor;
    class LayoutResource;
    class CameraTargetHolder {
    public:
        al::CameraTargetBase* getViewTarget(int) const;
    };

    void startHitReaction(const al::LiveActor*, const char*);
    void startAction(al::LiveActor*, const char*);
    void emitRadialBlur(const al::LiveActor*, const sead::Vector3f&, float, float, float, float, float, float, int, int,
                        bool);

    class SimpleLayoutAppearWaitEnd : public al::LayoutActor {
    public:
        SimpleLayoutAppearWaitEnd(char const*, char const*, al::LayoutInitInfo const&, char const*,
                                  bool);
        SimpleLayoutAppearWaitEnd(al::LayoutActor*, char const*, char const*, al::LayoutInitInfo const&,
                                  char const*);

        void appear() override;
        void end();
        void startWait();

        bool isAppearOrWait() const;
        bool isWait() const;

        void exeAppear();
        void exeWait();
        void exeEnd();

        int mWaitMaxStep;
    };

    class SimpleLayoutText : public al::LayoutActor {
    public:
        SimpleLayoutText(const al::LayoutInitInfo&, const char*, const char*, const char*);
        void start(const sead::Vector2f& offset, const char* text, int timer);
        void start(const sead::Vector2f& offset, const char16_t* text, int timer);

        void setText(const char* text);
    };
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
} // namespace

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