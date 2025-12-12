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
    PlayerActorHakoniwa* player = nullptr;
    struct SavedPlayerLocation {
      bool isSaved = false;
      sead::Vector3f position;
      sead::Quatf rotation;
    } lastPlayerLocation;
    u8 chosenCapture = 0;
    std::array<bool, 26> capturesChosen = {};
    al::LiveActor* capture = nullptr;
    sead::Vector3f lastCapturePos{};
    bool loadingSave = false;
    bool overrideResourceCategory = false;

    [[nodiscard]] sead::Vector3f calcPosition() const {
      sead::Vector3f front;
      al::calcFrontDir(&front, player);
      sead::Vector3f result = al::getTrans(player) + front * 500 + getActiveCaptureInfo().offset;
      if (auto calcOffset = getActiveCaptureInfo().calcOffset)
        calcOffset(result);
      return result;
    }

    [[nodiscard]] const CaptureInfo& getActiveCaptureInfo() const {
      return CaptureInfo::getCaptures()[chosenCapture];
    }

    static bool isAnagramAlphabetCharacter() {
      return al::isEqualString(instance()->getActiveCaptureInfo().gameName, "AnagramAlphabetCharacter");
    }

    static void init();
    void initSceneObj() override {}
    void initAfterPlacementSceneObj(const al::ActorInitInfo&) override;
    void update();
    void draw(sead::TextWriter*);
    void loadState();
    void saveState();

    [[nodiscard]] al::LiveActor* getMario() const { return player; }
    [[nodiscard]] al::LiveActor* getCapture() const { return capture; }
    bool isLoadingSave() const { return initialized() && loadingSave; }

    static bool initialized() { return sInstance; }
  };

  constexpr const char* stateFilePath = "sd:/capturemodstate.save";
} // namespace cs
