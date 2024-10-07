#include "al/Library/Area/AreaObj.h"
#include "al/Library/Area/AreaObjGroup.h"
#include "al/Library/Factory/Factory.h"
#include "al/Library/Nerve/NerveSetupUtil.h"
#include "al/Library/Nerve/NerveUtil.h"
#include "game/StageScene/StageSceneLayout.h"
#include "logger/Logger.hpp"
#include "math/seadVector.h"
#include "spook/FirstPersonState.hpp"
#include "spook/SePlayObjSpooky.hpp"
#include "types.h"
#include <al/Library/Audio/AudioDirector.h>
#include <al/Library/Controller/JoyPadUtil.h>
#include <al/Library/Light/ActorPrepassLightKeeper.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/LiveActor/ActorSensorFunction.h>
#include <al/Library/LiveActor/LiveActorGroup.h>
#include <al/Library/LiveActor/LiveActorKit.h>
#include <al/Library/Math/MathUtil.h>
#include <al/Library/Math/MathVectorUtil.h>
#include <al/Library/Player/PlayerHolder.h>
#include <al/Library/Resource/ResourceHolder.h>
#include <cstddef>
#include <game/GameData/GameDataFunction.h>
#include <game/Player/CapFunction.h>
#include <gfx/seadCamera.h>
#include <heap/seadHeapMgr.h>
#include <helpers/PlayerHelper.h>
#include <logger/Params.h>
#include <spook/SpookState.hpp>
#include <utils/ForwardDecls.hpp>
#include <utils/Helpers.h>

namespace sp {
  namespace {
    MAKE_NERVE_IMPL(SpookState, Normal);
    MAKE_NERVE_IMPL(SpookState, Flicker);
    MAKE_NERVE_IMPL(SpookState, Climb);
  } // namespace
  sead::LookAtCamera* camera = nullptr;
  void SpookState::initAfterPlacementSceneObj(const al::ActorInitInfo& initInfo) {
    if (!isSameType<StageScene>(scene))
      return;
    player = static_cast<PlayerActorHakoniwa*>(al::getPlayerActor(initInfo.mActorSceneInfo.mPlayerHolder, 0));
    auto& lights = player->mActorPrePassLightKeeper->mLights;
    flashlight = reinterpret_cast<al::LppSpot*>(lights[0]);
    flashlight->requestKillByUser();
    postProcessingFilter = unsafeRef<al::PostProcessingFilter*>(getGraphicsSystemInfo(initInfo), 0x2d0);
    Logger::log("Light count: %d/%d\n", lights.size(), lights.capacity());
    for (int i = 0; i < lights.size(); i++) {
      Logger::log("Mario's Light: %s %s %s\n", typeid(*lights[i]).name(), lights[i]->name->value,
                  BTOC(lights[i]->killedByUser), lights[i]);
    }

    firstPerson = alloc<FirstPersonState>(this, initInfo);
    camera = nullptr;
    audioDirector = initInfo.mAudioDirector;
    // spookyPlay = alloc<SePlayObjSpooky>(player);
    // al::initCreateActorNoPlacementInfo(spookyPlay, initInfo);

    scene->mGameDataHolder.mData->mDataFile->mIsEnableCap = true;
    player->mHackCap->hide(false);
    player->mPlayerAnimator->forceCapOn();

    // staticManager = alloc<StaticManager>(initInfo);
  }

  const struct Preset {
    const char* name;
    f32 red;
    f32 green;
    f32 blue;
    f32 degree;
    f32 length;
  } presets[3] = {
      {.name = "PresetStandard", .red = 10000, .green = 10000, .blue = 10000, .degree = 90, .length = 3000},
      {.name = "PresetDim", .red = 7500, .green = 7500, .blue = 7500, .degree = 45, .length = 1000},
      {.name = "PresetRed", .red = 10000, .green = 250, .blue = 250, .degree = 90, .length = 3000},
  };
  //   int curSound = 0;
  const char* sounds[] = {
      "LunaticPrincess",    "CreepyBreathing",    "RustyDoor",      "ScaryNoise1",      "ScaryNoise2",
      "ScaryNoise3",        "ScaryNoiseAmbience", "ScaryNoiseBell", "ScaryNoiseScream", "DriedBranches1",
      "MinecraftCaveSound", "WolfHowl1",          "WolfHowl2",
  };
  void SpookState::update() {
    if (par::clicked("EnableCappy")) {
      scene->mGameDataHolder.mData->mDataFile->mIsEnableCap = true;
      player->mHackCap->hide(false);
      player->mPlayerAnimator->forceCapOn();
    }

    // staticManager->update();

    if (!isNerve<StageSceneNrvPlay>(scene))
      return;

    updateAreaSpooky();

    if (!isSpookActive)
      return;

    if (al::isLessEqualStep(scene, 5)) {
      scene->mStageSceneLayout->end();
    }

    firstPerson->playerMovement();
    al::validatePostProcessingFilter(scene);
    postProcessingFilter->currentPreset = par::get("Filter", 0);
    if (postProcessingFilter->currentPreset > 17)
      postProcessingFilter->currentPreset = 17;

    player->mPlayerConst->mNormalMaxSpeed = par::get("MaxSpeed", 14.0f);
    if (!par::get("UpdateFlashlight", false)) {
      flashlight->requestKillByUser();
      return;
    }

    camera = al::getLookAtCamera(player, 0);

    if (par::get("BypassPresets", false)) {
      flashlight->currentColor = flashlight->targetColor = flashlight->color->value =
          sead::Color4f(par::get("ColorR", 10000.0f), par::get("ColorG", 8000.0f), par::get("ColorB", 9000.0f), 1.0f);
      flashlight->param.degree->value = par::get("Degree", 50.0f);
      flashlight->param.length->value = par::get("Length", 1000.0f);
    } else {
      auto& preset = presets[activePreset];
      flashlight->currentColor = flashlight->targetColor = flashlight->color->value =
          sead::Color4f(preset.red, preset.green, preset.blue, 1.0f);
      flashlight->param.degree->value = preset.degree;
      flashlight->param.length->value = preset.length;
    }
    flashlight->randomCeil->value = par::get("RandomCeil", 0.0f);
    flashlight->param.angleDamp->value = par::get("AngleDamp", 1.0f);
    flashlight->param.specularExpansion->value = par::get("SpecExpansion", 0.0f);
    flashlight->param.pcf->value = par::get("Pcf", 1.5f);
    auto* rotateOffset = &flashlight->rotateOffset->value;
    rotateOffset->x = par::get("RotateX", 0.0f);
    rotateOffset->y = par::get("RotateY", 0.0f);
    rotateOffset->z = par::get("RotateZ", 0.0f);
    flashlight->mtxConnector->init(player->getBaseMtx(), sead::Matrix34f::ident);

    if (par::clicked("NoBgm")) {
      al::stopAllBgm(player, 0);
    }

    if (al::isPadTriggerA(-1) || al::isPadTriggerB(-1)) {
      if (flashlight->killedByUser)
        flashlight->requestAppearByUser(0);
      else
        flashlight->requestKillByUser(0);
    }

    for (size_t i = 0; i < std::size(sounds); i++) {
      if (par::clicked(sounds[i])) {
        Logger::log("Playing sound from server %s\n", sounds[i]);
        alSeFunction::startSeFromUpperLayerSeKeeper(player, sounds[i]);
      }
    }

    for (s32 i = 0; i < std::size(presets); i++) {
      if (par::clicked(presets[i].name)) {
        if (flashlight->killedByUser) {
          activePreset = i;
        } else {
          nextPreset = i;
          al::setNerve(this, &SpookStateNrvFlicker::sInstance);
        }
      }
    }

    if (par::clicked("Flicker")) {
      al::setNerve(this, &SpookStateNrvFlicker::sInstance);
    }

    updateNerve();
  }

  void SpookState::exeFlicker() {
    if (al::isIntervalStep(this, 4, 0) && al::getRandom(5) < 2) {
      if (flashlight->killedByUser)
        flashlight->requestAppearByUser(0);
      else
        flashlight->requestKillByUser(0);
    }

    if (al::isGreaterEqualStep(this, 60)) {
      flashlight->requestKillByUser(0);
      al::setNerve(this, &SpookStateNrvClimb::sInstance);
    }
  }

  void SpookState::exeClimb() {
    constexpr s32 end = 120;
    if (al::isFirstStep(this)) {
      flashlight->requestAppearByUser(0);
    }
    auto t = static_cast<f32>(al::getNerveStep(this)) / static_cast<f32>(end);
    auto& preset = presets[activePreset];
    auto& newPreset = presets[nextPreset];
    flashlight->currentColor = flashlight->targetColor = flashlight->color->value =
        sead::Color4f(al::lerpValue(preset.red, newPreset.red, t), al::lerpValue(preset.green, newPreset.green, t),
                      al::lerpValue(preset.blue, newPreset.blue, t), 1.0f);
    flashlight->param.degree->value = al::lerpValue(preset.degree, newPreset.degree, t);
    flashlight->param.length->value = al::lerpValue(0, newPreset.length, t);
    if (al::isGreaterEqualStep(this, end)) {
      activePreset = nextPreset;
      Logger::log("updated preset! %d\n", activePreset);
      al::setNerve(this, &SpookStateNrvNormal::sInstance);
    }
  }

  void SpookState::startSpook() {
    Logger::log("Starting spook\n");
    isSpookActive = true;
    patch::CodePatcher patcher(0xa93450);
    patcher.Seek("_ZNK11PlayerInput25isTriggerCameraSubjectiveEv", 0x0);
    patcher.WriteInst(inst::MovRegister(reg::X0, reg::None64));
    patcher.WriteInst(inst::Ret());
    patcher.Seek("_ZN20PlayerActionFunction13isOppositeDirERKN4sead7Vector3IfEES4_", 0x0);
    patcher.WriteInst(inst::MovRegister(reg::X0, reg::None64));
    patcher.WriteInst(inst::Ret());
    patcher.Seek("_ZNK11PlayerInput13isTriggerJumpEv", 0x0);
    patcher.WriteInst(inst::MovRegister(reg::X0, reg::None64));
    patcher.WriteInst(inst::Ret());
    patcher.Seek("_ZNK11PlayerInput16isTriggerRollingEb", 0x14);
    patcher.WriteInst(inst::Movz(reg::W8, 1));
    patcher.Seek(0x4f02b0);
    ph::writeBooleanAndReturn(patcher, false);
    // scene->mGameDataHolder.mData->mDataFile->mIsEnableCap = false;
    CapFunction::putOnCapPlayer(player->mHackCap, player->mPlayerAnimator);
    flashlight->requestAppearByUser(0);
    scene->mStageSceneLayout->end();
    al::setNerve(this, &SpookStateNrvNormal::sInstance);
  }
  void SpookState::stopSpook() {
    Logger::log("Stopping spook\n");
    isSpookActive = false;
    patch::CodePatcher patcher(0xa93450);
    patcher.Seek("_ZNK11PlayerInput25isTriggerCameraSubjectiveEv", 0x0);
    patcher.Write(0xF81E0FF3);
    patcher.Write(0xA9017BFD);
    patcher.Seek("_ZN20PlayerActionFunction13isOppositeDirERKN4sead7Vector3IfEES4_", 0x0);
    patcher.Write(0x2D400420);
    patcher.Write(0xD000A348);
    patcher.Seek("_ZNK11PlayerInput13isTriggerJumpEv", 0x0);
    patcher.Write(0xA9BE4FF4);
    patcher.Write(0xA9017BFD);
    patcher.Seek("_ZNK11PlayerInput16isTriggerRollingEb", 0x14);
    patcher.Write(0x39426268);
    // scene->mGameDataHolder.mData->mDataFile->mIsEnableCap = true;
    GameDataFunction::disableCapByPlacement(player);
    player->mHackCap->hide(false);
    player->mPlayerAnimator->forceCapOn();
    firstPerson->disable();
    flashlight->requestKillByUser(0);
    al::invalidatePostProcessingFilter(scene);
    player->mPlayerConst->mNormalMaxSpeed = 14.0f;
    scene->mStageSceneLayout->start();
  }

  void SpookState::updateAreaSpooky() {
    if (par::get("OverrideSpooky", false)) {
      if (par::get("SpookyActive", false) != isSpookActive)
        toggleArea();
      return;
    }

    al::AreaObjGroup* area = al::tryFindAreaObjGroup(player, "SpookyArea");
    if (area == nullptr) {
      if (isSpookActive)
        stopSpook();
      return;
    }

    for (int i = 0; i < area->mCount; i++) {
      al::AreaObj* s = area->getAreaObj(i);
      if (s->isInVolume(al::getTrans(player)) != isSpookActive) {
        toggleArea();
        return;
      }
    }
  }

  void SpookState::toggleArea() {
    if (!isSpookActive)
      startSpook();
    else
      stopSpook();
  }

  struct StageStateCreate : public Trampoline<StageStateCreate> {
    static al::SceneObjHolder* Callback(al::Scene* scene, al::SceneObjHolder* holder) {
      Orig(scene, holder);

      sead::ScopedCurrentHeapSetter setter(al::getSceneHeap());
      auto state = alloc<SpookState>();
      if (isSameType<StageScene>(scene))
        state->scene = static_cast<StageScene*>(scene);
      holder->setSceneObj(state, 0x40);
      return holder;
    }
  };

  struct StageScenePlay : public Trampoline<StageScenePlay> {
    static void Callback(StageScene* scene) {
      Orig(scene);

      spookyState(scene).update();
    }
  };

  struct CheckAddonLoadStates : public Trampoline<CheckAddonLoadStates> {
    static bool Callback(s32 type, const char* name, al::SeadAudioPlayer* player) {
      bool res = Orig(type, name, player);
      Logger::log("%s: %s\n", name, BTOC(res));
      return res;
    }
  };

  struct AreaConvertName : exl::hook::impl::TrampolineHook<AreaConvertName> {
    static const char* Callback(al::Factory<al::AreaObj>* factory, const char* name) {
      if (al::isEqualString(name, "SpookyArea"))
        return "GpuPerfArea";
      return Orig(factory, name);
    }
  };

  void spookyInit() {
    StageStateCreate::InstallAtSymbol("_ZN2al5Scene18initSceneObjHolderEPNS_14SceneObjHolderE");
    StageScenePlay::InstallAtSymbol("_ZN10StageScene7controlEv");
    CheckAddonLoadStates::InstallAtOffset(0x808cc0);

    struct PlayerReceiveMsg : Trampoline<PlayerReceiveMsg> {
      static bool Callback(al::LiveActor* actor, al::HitSensor* left, al::HitSensor* right) {
        if (spookyState(actor).isSpookActive && isSameType<PuppetActor>(al::getSensorHost(left)) &&
            isSameType<PuppetActor>(al::getSensorHost(right))) {
          Logger::log("Spook received PuppetActor\n");
          PlayerHelper::killPlayer(actor);
          return true;
        }
        return Orig(actor, left, right);
      }
    };
    PlayerReceiveMsg::InstallAtSymbol("_ZN19PlayerActorHakoniwa10receiveMsgEPKN2al9SensorMsgEPNS0_9HitSensorES5_");
    patch::CodePatcher patcher(0xa93450);
    patcher.Write(inst::Movz(W0, 1));
    patcher.Seek(0xa582c8);
    patcher.Write(inst::Movz(W8, 1));

    patcher.Seek(0x97bd4c);
    patcher.Write(0x52B85648); // -30 -> -89 for min angle
    patcher.SeekRel(16);
    patcher.Write(0x52A85648); // 75 -> 89 for max angle
    patcher.SeekRel(4);
    patcher.Write(0x52A86188); // 180 -> 110 for height

    patcher.Seek(0x97c0a4); // disable * 50f for CameraPoserSubjective offset
    patcher.WriteInst(inst::Nop());
    patcher.WriteInst(inst::Nop());
    patcher.WriteInst(inst::Nop());
    patcher.WriteInst(inst::Nop());

    AreaConvertName::InstallAtSymbol("_ZNK2al7FactoryIPFPNS_7AreaObjEPKcEE11convertNameES4_");
  }

  // void spookyCreate() { StaticManager::init(); }
} // namespace sp
