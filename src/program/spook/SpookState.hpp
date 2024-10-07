#pragma once

#include "StaticManager.hpp"
#include "al/Library/Bgm/BgmDirector.h"
#include "al/Library/Se/SeDirector.h"
#include "spook/SePlayObjSpooky.hpp"
#include <al/Library/Scene/ISceneObj.h>
#include <game/Player/PlayerActorHakoniwa.h>
#include <game/StageScene/StageScene.h>
#include <spook/FirstPersonState.hpp>
#include <utils/ForwardDecls.hpp>

namespace sp {
  namespace {
    MAKE_NERVE_BASE(SpookState, Normal);
    MAKE_NERVE_BASE(SpookState, Flicker);
    MAKE_NERVE_BASE(SpookState, Climb);
  } // namespace
  struct SpookState : al::ISceneObj, al::NerveExecutor {
    PlayerActorHakoniwa* player = nullptr;
    StageScene* scene = nullptr;
    al::PostProcessingFilter* postProcessingFilter = nullptr;
    al::LppSpot* flashlight = nullptr;
    al::LiveActor* follower = nullptr;
    FirstPersonState* firstPerson = nullptr;
    sead::Matrix34f noRotateMtx = {};
    al::AudioDirector* audioDirector = nullptr;
    SePlayObjSpooky* spookyPlay = nullptr;
    bool isSpookActive = false;
    s32 activePreset = 0;
    s32 nextPreset = 0;
    StaticManager* staticManager = nullptr;
    sead::PtrArray<al::LiveActor*>* puppets = nullptr;

    SpookState() : al::NerveExecutor("SpookState") { initNerve(&SpookStateNrvNormal::sInstance, 0); }
    void initAfterPlacementSceneObj(const al::ActorInitInfo&) override;
    void update();
    void startSpook();
    void stopSpook();

    void updateAreaSpooky();
    void toggleArea();

    void exeNormal() const {}
    void exeFlicker();
    void exeClimb();
  };

  [[maybe_unused]] static SpookState& spookyState(al::IUseSceneObjHolder* sceneObjHolder) {
    return *(SpookState*)al::getSceneObj(sceneObjHolder, 0x40);
  }

  void spookyInit();
} // namespace sp
