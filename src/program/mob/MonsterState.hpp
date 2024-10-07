#pragma once

#include "camera/FreecamPoser.hpp"
#include "game/Player/PlayerActorHakoniwa.h"
#include "mob/MonsterInfo.hpp"
#include <al/Library/LiveActor/ActorInitInfo.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/Scene/Scene.h>
#include <game/StageScene/StageScene.h>
#include <heap/seadHeapMgr.h>
#include <logger/Params.h>

namespace mob {
  constexpr int TRACKED_OFFSPRING = 256;
  struct MonsterState final : al::ISceneObj {
    SEAD_SINGLETON_DISPOSER(MonsterState)
    ~MonsterState() override = default;
    MonsterState() = default;

public:
    StageScene* scene = nullptr;
    PlayerActorHakoniwa* player = nullptr;
    PoserFreecam* freecam = nullptr;
    al::CameraTicket* freecamTicket = nullptr;
    int startState = 10;
    int firstMonster = 0;
    int secondMonster = 1;
    sead::Vector3f startPos;
    sead::Vector3f firstPos;
    sead::Vector3f secondPos;
    al::LiveActor* firstActor = nullptr;
    sead::FixedPtrArray<al::LiveActor, TRACKED_OFFSPRING> firstOffspring{};
    al::LiveActor* firstTarget = nullptr;
    al::LiveActor* secondActor = nullptr;
    sead::FixedPtrArray<al::LiveActor, TRACKED_OFFSPRING> secondOffspring{};
    al::LiveActor* secondTarget = nullptr;
    al::LiveActor* currentlyInitializing = nullptr;
    al::LiveActor* firstSpawn;
    al::LiveActor* secondSpawn;

    bool isAMonster(const al::LiveActor* actor) const {
      return actor == firstActor || actor == secondActor || isOffspring(actor, firstActor) ||
             isOffspring(actor, secondActor) || actor == firstSpawn || actor == secondSpawn;
    }
    bool isFirstMonster(const al::LiveActor* actor) const {
      return actor == firstActor || actor == firstSpawn || isOffspring(actor, firstActor);
    }
    bool isOffspring(const al::LiveActor* actor, const al::LiveActor* parent) const {
      if (const auto offspring = getOffspringConst(parent)) {
        //         if (offspring->indexOf(actor) >= 0)
        // Logger::log("offspring check success! %s\n", typeid(*actor).name());
        return offspring->indexOf(actor) >= 0;
      }
      return false;
    }
    static al::LiveActor* staticGetEnemy(const al::LiveActor* actor) {
      // Logger::log("ayo static checker %p %p\n", actor, instance()->getEnemy(actor));
      return instance()->getEnemy(actor);
    }

    sead::FixedPtrArray<al::LiveActor, TRACKED_OFFSPRING>* getOffspring(const al::LiveActor* actor) {
      if (!isAMonster(actor))
        return nullptr;
      return isFirstMonster(actor) ? &firstOffspring : &secondOffspring;
    }

    const sead::FixedPtrArray<al::LiveActor, TRACKED_OFFSPRING>* getOffspringConst(const al::LiveActor* actor) const {
      if (!isAMonster(actor))
        return nullptr;
      return isFirstMonster(actor) ? &firstOffspring : &secondOffspring;
    }

    [[nodiscard]] sead::Vector3f calcPosition(const al::LiveActor* actor) const {
      sead::Vector3f result = isFirstMonster(actor) ? firstPos : secondPos;
      if (const auto calcOffset = getActiveMonsterInfo(actor).calcOffset)
        calcOffset(result);
      return result;
    }

    al::LiveActor* getEnemyClean(const al::LiveActor* actor) const {
      return isFirstMonster(actor) ? (secondTarget ? secondTarget : secondActor)
                                   : (firstTarget ? firstTarget : firstActor);
    }

    al::LiveActor* getEnemy(const al::LiveActor* actor) const {
      return isFirstMonster(actor) ? secondSpawn : firstSpawn;
    }

    sead::Vector3f& calcEnemyPosition(const al::LiveActor* actor) {
      if (currentlyInitializing) {
        bool isFirst = isFirstMonster(currentlyInitializing);
        sead::Vector3f result = isFirst ? firstPos : secondPos;
        if (const auto calcOffset = getActiveMonsterInfo(isFirst).calcOffset)
          calcOffset(result);
        (isFirst ? firstPos : secondPos) = result;
        return isFirst ? firstPos : secondPos;
      }

      return getEnemy(actor)->getPoseKeeper()->mTrans;
    }

    void setTarget(al::LiveActor* actor) {
      auto& target = isFirstMonster(actor) ? secondTarget : firstTarget;
      target = actor;
    }

    const MonsterInfo& getActiveMonsterInfo(const bool first) const {
      return MonsterInfo::getMonsters()[first ? firstMonster : secondMonster];
    }

    const MonsterInfo& getActiveMonsterInfo(const al::LiveActor* actor) const {
      return getActiveMonsterInfo(isFirstMonster(actor));
    }

    bool canAffect(const al::LiveActor* self, const al::LiveActor* other) const {
      return !isAMonster(self) || !isAMonster(other) || isFirstMonster(self) != isFirstMonster(other);
    }

    static void init();
    void update();
    void spawnMonster(al::LiveActor*& actor, const MonsterInfo& info, const al::ActorInitInfo& initInfo);

    void initAfterPlacementSceneObj(al::ActorInitInfo const&) override;
  };
} // namespace mob
