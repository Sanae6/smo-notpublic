#pragma once

#include "al/Library/Audio/IUseAudioKeeper.h"
#include "heap/seadHeap.h"
#include "prim/seadSafeString.h"
#include <agl/TextureData.h>
#include <agl/gpu.h>
#include <agl/util.h>
#include <agl/utl.h>
#include <al/Library/Camera/CameraTargetBase.h>
#include <al/Library/Layout/LayoutActor.h>
#include <al/Library/LiveActor/ActorModelFunction.h>
#include <al/Library/LiveActor/LiveActor.h>
#include <al/Library/Nerve/NerveSetupUtil.h>
#include <al/Library/Scene/Scene.h>
#include <container/seadPtrArray.h>
#include <container/seadStrTreeMap.h>
#include <container/seadTList.h>
#include <gfx/gfx_DescriptorSlot.h>
#include <gfx/gfx_Device.h>
#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>

class CapTargetInfo;
class IUsePlayerHack;

namespace eui {
  bool RegisterSlotForTexture(nn::gfx::DescriptorSlot*, nn::gfx::TTextureView<nn::gfx::ApiVariationNvn8> const&, void*);
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
      bool SetResource(gfx::Device* device, void* fontData, gfx::TMemoryPool<gfx::ApiVariationNvn8>*,
                       ptrdiff_t memoryPoolOffset, size_t memoryPoolSize);
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
  namespace fs {
    Result CommitSaveData(const char* name);
  }
  namespace ui2d {
    class TextureInfo;
  }
} // namespace nn
namespace al {
  template <class T>
  al::LiveActor* createActorFunction(const char* name);

  f32 getRandom();
  f32 getRandom(f32);
  f32 getRandom(f32, f32);
  s32 getRandom(s32);
  s32 getRandom(s32, s32);
  void getRandomVector(sead::Vector3f*, f32);
  void snapVecToGrid(sead::Vector3f* output, const sead::Vector3f& position, f32 cubeSize,
                     const sead::Vector3f& offset);

  class OneMeshFixMapParts {};
  class LiveActor;
  class LayoutResource;
  class CameraTargetHolder {
public:
    al::CameraTargetBase* getViewTarget(int) const;
  };

  void startHitReaction(const al::LiveActor*, const char*);
  void startAction(al::LiveActor*, const char*);
  void startSe(const al::IUseAudioKeeper* user, const sead::SafeString& name);
  al::CameraTicket* initSubjectiveCameraNoSave(al::IUseCamera const*, char const*);
  bool isActiveCamera(al::CameraTicket const*);
  void startCamera(al::IUseCamera const*, al::CameraTicket*, int);
  void endCamera(al::IUseCamera const*, al::CameraTicket*, int, bool);
  void calcCameraLookDir(sead::Vector3f*, al::IUseCamera const*, int);

  void emitRadialBlur(const al::LiveActor*, const sead::Vector3f&, float, float, float, float, float, float, int, int,
                      bool);
  void validatePostProcessingFilter(const al::Scene* scene);
  void invalidatePostProcessingFilter(const al::Scene* scene);

  void lerpVec(sead::Vector3f*, sead::Vector3f const&, sead::Vector3f const&, f32);

  class SimpleLayoutAppearWaitEnd : public al::LayoutActor {
public:
    SimpleLayoutAppearWaitEnd(char const*, char const*, al::LayoutInitInfo const&, char const*, bool);
    SimpleLayoutAppearWaitEnd(al::LayoutActor*, char const*, char const*, al::LayoutInitInfo const&, char const*);

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

  class PostProcessingFilter {
public:
    bool isValid;
    class ShaderHolder* shaderHolder;
    class DepthOfFieldDrawer* depthOfFieldDrawer;
    class GraphicsParamRequesterImpl* graphicsParamRequesterImpl;
    class ViewDepthDrawer* viewDepthDrawer;
    class VignettingDrawer* vignettingDrawer;
    class EdgeDrawer* edgeDrawer;
    class CartoonDrawer* cartoonDrawer;
    class RetroColorDrawer* retroColorDrawer;
    class ScreenBlurDrawer* screenBlurDrawer;
    class PencilSketchDrawer* pencilSketchDrawer;
    class ColorClampDrawer* colorClampDrawer;
    sead::PtrArray<struct PostProcessingFilterPreset> filterPresets;
    s32 currentPreset;
  };

  SEAD_ENUM(YamlParamType, Invalid, Bool, F32, S32, U32, V2f, V2s32, V3f, V4f, Q4f, C4f, StringRef, String32, String64,
            String128, String256, String512, String1024, String2048, String4096);

  class ParameterBase {
private:
    al::ParameterBase* nextParam;
    sead::FixedSafeString<64> name;
    u32 nameHash;

public:
    virtual const char* getParamTypeStr() const = 0;
    virtual YamlParamType getParamType() const = 0;
    virtual void* ptr() const = 0;
    virtual void* ptr() = 0;
    virtual void afterGetParam();
    virtual u32 size() = 0;
    virtual bool isEqual(const al::ParameterBase&);
    virtual bool copy(const al::ParameterBase&);
    virtual bool copyLerp(const al::ParameterBase&, f32 t);
  };

  template <typename T>
  class Parameter : ParameterBase {
private:
    T value;

public:
    YamlParamType getParamType() const override;
    const char* getParamTypeStr() const override;
    void* ptr() const override;
    void* ptr() override;
    u32 size() override;

    T& ref() { return value; }
  };

  class ParameterF32 : public Parameter<f32> {
public:
    YamlParamType getParamType() const override;
    const char* getParamTypeStr() const override;
  };

  class ParameterS32 : public Parameter<s32> {
public:
    YamlParamType getParamType() const override;
    const char* getParamTypeStr() const override;
  };

  class ParameterBool : public Parameter<bool> {
public:
    YamlParamType getParamType() const override;
    const char* getParamTypeStr() const override;
  };

  class ParameterStringRef : public Parameter<const char*> {
    YamlParamType getParamType() const override;
    const char* getParamTypeStr() const override;
  };

  class ParameterV3f : public Parameter<sead::Vector3f> {

    YamlParamType getParamType() const override;
    const char* getParamTypeStr() const override;
  };

  class ParameterC4f : public Parameter<sead::Color4f> {
    YamlParamType getParamType() const override;
    const char* getParamTypeStr() const override;
  };

  class ParameterObj;
  class MtxConnector {
public:
    sead::Matrix34f baseMtx;
    sead::Matrix34f* targetMtx;
    sead::Quatf baseQuat;
    sead::Vector3f baseTrans;

    virtual bool isConnecting() const;
    virtual void clear();
    void init(sead::Matrix34f const*);
    void init(sead::Matrix34f const*, sead::Matrix34f const&);
    void setBaseQuatTrans(sead::Quatf const&, sead::Vector3f const&);
    void calcConnectInfo(sead::Vector3f*, sead::Quatf*, sead::Vector3f*, sead::Vector3f const&,
                         sead::Vector3f const&) const;
  };

  class PrePassLightBase : public al::NerveExecutor,
                           public sead::TListNode<al::PrePassLightBase*>,
                           public IUseCollision {
private:
    al::GraphicsSystemInfo* graphicsSystemInfo;
    al::CollisionDirector* collisionDirector;
    al::ParameterObj* parameterObj;
    al::ParameterStringRef* name;
    al::ParameterV3f* offset;
    al::ParameterV3f* rotateOffset;
    al::ParameterC4f* color;
    al::ParameterBool* isEnableSpecular;
    al::ParameterS32* killFrame;
    al::ParameterS32* appearFrame;
    al::ParameterBool* isIndirectIllumination;
    al::ParameterF32* randomCeil;
    al::MtxConnector* mtxConnector;
    bool overrideUserColor;
    sead::Color4f userColor;
    bool killedByUser;
    sead::Color4f currentColor;
    sead::Color4f targetColor;
    s32 appearTargetFrame;
    s32 killTargetFrame;
    sead::Random* random;

public:
    void requestAppearByUser(s32 appearTime = -1);
    void requestKillByUser(s32 killTime = -1);
  };
  template <typename T>
  class PrePassLight : public PrePassLightBase {
private:
    T param;
  };
  struct LppSpotParam {
    ParameterF32* degree;
    ParameterF32* length;
    ParameterF32* angleDamp;
    ParameterF32* specularExpansion;
    ParameterBool* isEnableCollisionCheck;
    ParameterF32* afterCollisionCheckOffset;
    ParameterF32* lengthChangeRate;
    ParameterS32* useShadow;
    ParameterF32* pcf; // percentage-closer filtering
  };
  class LppSpot : public PrePassLight<LppSpotParam> {};
  template <typename T>
  class PrePassLightPlacementBase : public al::LiveActor {
private:
    T* prePassLight;
    sead::Matrix34f matrix;
    sead::Vector3f someVec;
    f32 someFloat;
  };
  class LppSpot;
  class PrePassSpotLight : public PrePassLightPlacementBase<LppSpot> {
    PrePassSpotLight();
  };
  void setPrePassLightOffset(const al::LiveActor*, const char* lightName, const sead::Vector3f&);
  void requestPrePassLightColor(const al::LiveActor*, const char* lightName, const sead::Color4f& color);
  void requestPrePassLightColor(const al::LiveActor*, const char* lightName, float multiplier);
  void requestPrePassLightColor(const al::LiveActor*, const char* lightName, const char* colorName, float multiplier);

  void attachMtxConnectorToMtxPtr(al::MtxConnector*, const sead::Matrix34f*);
  sead::LookAtCamera* getLookAtCamera(const al::IUseCamera*, s32 index);
  sead::Projection* getProjectionSead(al::IUseCamera const*, int);

  class SePlayParamList;
  struct MeInfo;
  class SeKeeper {
public:
    void requestPlaySe(const char*, float, const char*, const al::SePlayParamList*, const al::MeInfo*, bool,
                       const char*, const sead::Vector3f*);
    bool checkIsPlayingSe(const char* name, const char* unk) const;
  };

  void startSe(const al::IUseAudioKeeper*, const sead::SafeString&);
  void stopAllBgm(const al::IUseAudioKeeper*, s32);
  class AreaObjGroup;
  al::AreaObjGroup* tryFindAreaObjGroup(const al::IUseAreaObj*, const char*);
  u8* loadFile(sead::SafeString const&, s32 alignment);

  void setCameraTarget(al::IUseCamera*, al::CameraTargetBase*);
  void resetCameraTarget(al::IUseCamera*, al::CameraTargetBase*);

  bool isOnGround(const al::LiveActor* actor, u32);

  bool isCollidedGround(const al::LiveActor* actor);

  void initLayoutActor(al::LayoutActor*, const al::LayoutInitInfo&, const char*, const char*);

  nn::ui2d::TextureInfo* createTextureInfo();
  nn::ui2d::TextureInfo* createTextureInfo(const agl::TextureData&);
  void setPaneTexture(al::IUseLayout*, const char*, const nn::ui2d::TextureInfo*);
  void updateTextureInfo(nn::ui2d::TextureInfo*, agl::TextureData const&);

  void startAction(al::IUseLayoutAction*, char const*, char const*);

  struct TextureUnit {
    agl::TextureData* textureData;
    agl::TextureSampler* textureSampler;
    u8 _unused[0x18];
    sead::FixedSafeString<64> name;
  };
  struct NoiseTexture {
    TextureUnit* data;
    agl::UniformBlock* uniformBlock;
    f32 floats[7];
  };
  struct NoiseTextureKeeper {
    u8 _partsGraphics[0x28];
    NoiseTexture* black2d;
    NoiseTexture* black3d;
    void* curlShaderProgram;
    NoiseTexture* curl2d;
    NoiseTexture* curl3d;
    void* simpleShaderProgram;
    NoiseTexture* simple;

    NoiseTextureKeeper();

    void declareUsingSimpleNoiseTexture();
  };

  agl::TextureData& getBlack2DTexture();

  struct FullScreenQuadModel {
    u8 implementation[0x220];
    FullScreenQuadModel();
    ~FullScreenQuadModel();
    void drawQuad(agl::DrawContext*) const;
  };

  char16_t* getSystemMessageString(al::IUseMessageSystem const*, char const*, char const*);

  struct ResourceCategory {
    sead::FixedSafeString<128> name;
    sead::Heap* heap;
    sead::StrTreeMap<156, al::Resource*> aa;
  };
  class ResourceSystem {
public:
    ResourceCategory** categories;
    int a[2];
    int count;
    void findResourceCategory(sead::SafeStringBase<char> const&);
  };
} // namespace al
namespace rs {
  bool isActiveDemo(const al::LiveActor* actor);
  bool isPlayer2D(const al::LiveActor* actor);
  bool isPlayerInWater(const al::LiveActor*);
  IUsePlayerHack* startHack(al::HitSensor*, al::HitSensor*, al::LiveActor* actor = nullptr);
  void endHack(IUsePlayerHack** hack);
  CapTargetInfo* createCapTargetInfo(al::LiveActor*, const char*);
  bool isTriggerUiDecide(al::IUseSceneObjHolder const*);
} // namespace rs
namespace alAudioKeeperFunction {
  al::AudioKeeper* createAudioKeeper(const al::AudioDirector* director);
}
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
    WallDamage = 10,
  };
  void set(EActionTrigger trigger);
  bool isOn(EActionTrigger trigger) const;
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
  void shoot(const sead::Vector3f&, const sead::Vector3f&, int, bool, bool);
  void shootByPlayer(const sead::Vector3f& trans, const sead::Vector3f& vel, const sead::Vector3f& dir,
                     const sead::Vector3f&, float, int);

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

namespace alSeFunction {
  void startSeFromUpperLayerSeKeeper(const al::IUseAudioKeeper* keeper, const char* name);
}
class MapMini : public al::LayoutActor {
  public:
  MapMini(al::LayoutInitInfo const&, al::PlayerHolder const*);
  void appearSlideIn(void);
  void end(void);
  void calcNearHintTrans(void);

  bool isEnd(void) const;

  void exeAppear(void);
  void exeWait(void);
  void exeEnd(void);
};

namespace al {
  class HitSensor;

  class HitSensorKeeper {
public:
    HitSensorKeeper(s32);

    HitSensor* getSensor(const char*) const;
    HitSensor* getSensor(s32) const;
    s32 getSensorNum() const;

public:
    s32 mSensorCount;
    HitSensor** mSensors;
  };
} // namespace al

class HackerJudgeNormalFall;
class HackerJudgeStartRun;
class CapTargetInfo;
class AnagramAlphabet;
class IUsePlayerHack;
class CapTargetParts;
class PlayerHackStartShaderCtrl;

class AnagramAlphabetCharacter : public al::LiveActor {
  public:
  AnagramAlphabetCharacter(const char*);

  void init(const al::ActorInitInfo&);
  void attackSensor(al::HitSensor*, al::HitSensor*);
  bool receiveMsg(const al::SensorMsg*, al::HitSensor*, al::HitSensor*);
  void setComplete();
  void killCapTarget();

  void exeWait();
  void exeWaitHack();
  void exeWaitHackStart();
  void exeHackStart();
  void exeHackWait();
  void exeHackMove();
  void exeHackFall();
  void exeHackEnd();
  void exeHackGoal();
  void exeSet();
  void exeComplete();

  private:
  CapTargetInfo* mCapTargetInfo;
  sead::Matrix34f* unkMtx;
  AnagramAlphabet* mParent;
  IUsePlayerHack* mHackerParent;
  CapTargetParts* mCapTargetParts;
  HackerJudgeNormalFall* mHackerJudgeNormalFall;
  HackerJudgeStartRun* mHackerJudgeStartRun;
  PlayerHackStartShaderCtrl* mPlayerHackStartShaderCtrl;
  s32 mSwingTimer;
};

class TalkNpc : private al::LiveActor {};
struct PlayerStartObj : private al::LiveActor {
  struct IUsePlayerPuppet* puppet;
  bool isDemoObj;
  bool wasMissInPrevStage;
  sead::FixedSafeString<128> startId;
};
namespace alCameraFunction {
  al::CameraTicket* initCamera(al::CameraPoser*, al::IUseCamera const*, al::ActorInitInfo const&, char const*, int);
}

struct PuppetActor : al::LiveActor {};
struct MoviePlayer : al::ISceneObj {
  MoviePlayer();
  agl::TextureData& getTexture() const;
  void play(const char* path);
  void update();
};
