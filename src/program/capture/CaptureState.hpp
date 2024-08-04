#pragma once

#include "CaptureInfo.hpp"
#include "game/Player/PlayerActorHakoniwa.h"
#include <al/Library/LiveActor/ActorInitInfo.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <heap/seadHeapMgr.h>
#include <logger/Params.h>
#include <textwriter.h>

namespace cs {

  struct CaptureState final : al::ISceneObj {
    SEAD_SINGLETON_DISPOSER(CaptureState)
    ~CaptureState() final = default;
    CaptureState() = default;

public:

    s32 chosenCapture = 0;
    PlayerActorHakoniwa* player = nullptr;
    al::LiveActor* capture = nullptr;
    sead::Vector3f lastCapturePos{};
    bool loadingSave = false;

    [[nodiscard]] sead::Vector3f calcPosition() const {
      sead::Vector3f front;
      al::calcFrontDir(&front, player);
      sead::Vector3f result = al::getTrans(player) + front * 500 + getActiveCaptureInfo().offset;
      if (auto calcOffset = getActiveCaptureInfo().calcOffset)
        calcOffset(result);
      return result;
    }

    [[nodiscard]] static const CaptureInfo& getActiveCaptureInfo() {
      s32 value = par::get("SelectedCapture", 0);
      if (value >= CaptureInfo::getCaptures().size())
        value = (s32) CaptureInfo::getCaptures().size() - 1;
      return CaptureInfo::getCaptures()[value];
    }

    static bool isAnagramAlphabetCharacter() {
      return al::isEqualString(getActiveCaptureInfo().gameName, "AnagramAlphabetCharacter");
    }

    static void init();
    void initSceneObj() override {}
    void initAfterPlacementSceneObj(const al::ActorInitInfo&) override;
    void update();
    void draw(sead::TextWriter*);

    [[nodiscard]] al::LiveActor* getMario() const { return player; }
    [[nodiscard]] al::LiveActor* getCapture() const { return capture; }
    bool isLoadingSave() const { return initialized() && loadingSave; }

    static bool initialized() { return sInstance; }
  };

} // namespace cs
