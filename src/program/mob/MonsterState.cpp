#include "MonsterState.hpp"

#include "MonsterAttackHooks.hpp"

#include <al/Library/LiveActor/ActorFlagFunction.h>
#include <al/Library/LiveActor/ActorModelFunction.h>
#include <al/Library/LiveActor/ActorMovementFunction.h>
#include <al/Library/LiveActor/LiveActorGroup.h>
#include <al/Library/LiveActor/LiveActorKit.h>
#include <al/Library/Nerve/NerveStateBase.h>
#include <al/Library/Placement/PlacementFunction.h>
#include <al/Library/Player/PlayerHolder.h>
#include <al/Library/Resource/ResourceHolder.h>
#include <al/Library/Scene/Scene.h>
#include <game/GameData/GameDataFunction.h>
#include <game/HakoniwaSequence/HakoniwaSequence.h>
#include <game/StageScene/StageSceneLayout.h>
#include <game/System/GameSystem.h>
#include <helpers/PlayerHelper.h>
#include <rs/util/SensorUtil.h>
#include <utils/ForwardDecls.hpp>
#include <utils/Helpers.h>

namespace mob {
  SEAD_SINGLETON_DISPOSER_IMPL(MonsterState);
  void internalDestroyResource() {
    auto* seq = static_cast<HakoniwaSequence*>(GameSystemFunction::getGameSystem()->mCurSequence);
    if (!seq)
      return;

    WorldResourceLoader* res = seq->mResourceLoader;

    if (res->mCapWorldHeap) {
      al::removeResourceCategory("ワールド常駐");
      al::removeNamedHeap("WorldResourceCap");
      res->mCapWorldHeap = nullptr;
    }

    if (res->mWaterfallWorldHeap) {
      al::removeResourceCategory("ホーム常駐[Waterfall]");
      al::removeNamedHeap("WorldResourceWaterfall");
      res->mWaterfallWorldHeap = nullptr;
    }

    al::destroyWorldResourceHeap(!res->mCapWorldHeap);
    res->mWorldResourceHeap = nullptr;
    res->mLoadWorldId = -1;

    al::resetCurrentCategoryName();
  }

  void MonsterState::init() {

    struct StateCreationHook : Trampoline<StateCreationHook> {
      static al::SceneObjHolder* Callback(StageScene* scene, al::SceneObjHolder* holder) {
        Orig(scene, holder);
        MonsterState::instance()->scene = scene;
        if (isSameType<StageScene>(scene))
          holder->setSceneObj(MonsterState::instance(), 0x40);
        return holder;
      }
    };

    struct StateUpdateHook : Trampoline<StateUpdateHook> {
      static void Callback(StageScene* scene) {
        instance()->update();
        Orig(scene);
      }
    };
    struct ActorInit : Trampoline<ActorInit> {
      static void Callback(al::LiveActor* actor, const al::ActorInitInfo& initInfo) {
        Orig(actor, initInfo);

        const char* waow = nullptr;
        if (al::tryGetStringArg(&waow, initInfo, "ChangeStageId")) {
          Logger::log("found spawn obj during init %s\n", waow);
          if (al::isEqualString(waow, "Enemy_1")) {
            instance()->firstSpawn = actor;
            instance()->firstPos = al::getTrans(actor);
          }
          if (al::isEqualString(waow, "Enemy_2")) {
            instance()->secondSpawn = actor;
            instance()->secondPos = al::getTrans(actor);
          }
        }
        if (instance()->currentlyInitializing == actor) {
          al::setTrans(actor, instance()->calcPosition(actor));
        }

        if (instance()->currentlyInitializing && instance()->currentlyInitializing != actor) {
          const auto offspring = instance()->getOffspring(instance()->currentlyInitializing);
          Logger::log("Adding %s: %d/%d\n", typeid(*actor).name(), offspring->size(), offspring->capacity());
          offspring->pushBack(actor);
        }

        if (instance()->isAMonster(actor)) {
          al::invalidateClipping(actor);
          actor->mFlags->isDrawClipped = true;
        }
      }
    };

    struct ActorInitSRT : Trampoline<ActorInitSRT> {
      static void Callback(al::LiveActor* actor, const al::ActorInitInfo& initInfo) {
        Orig(actor, initInfo);
        if (instance()->currentlyInitializing) {
          Logger::log("Updating postition for bitch %s\n", typeid(*actor).name());
          al::setTrans(actor, instance()->calcPosition(actor));
        }
      }
    };

    struct TryFindNearestPlayerActor : Trampoline<TryFindNearestPlayerActor> {
      static al::LiveActor* Callback(al::LiveActor* actor) {
        if (instance()->isAMonster(actor))
          return instance()->getEnemy(actor);

        return Orig(actor);
      }
    };
    TryFindNearestPlayerActor::InstallAtSymbol("_ZN2al25tryFindNearestPlayerActorEPKNS_9LiveActorE");

    struct GetPlayerBodyPos : Trampoline<GetPlayerBodyPos> {
      static const sead::Vector3f& Callback(al::LiveActor* actor) {
        if (instance()->isAMonster(actor))
          return instance()->calcEnemyPosition(actor);

        return Orig(actor);
      }
    };
    GetPlayerBodyPos::InstallAtSymbol("_ZN2rs16getPlayerBodyPosEPKN2al9LiveActorE");

    struct GetPlayerPosRs : Trampoline<GetPlayerPosRs> {
      static const sead::Vector3f& Callback(al::LiveActor* actor) {
        if (instance()->isAMonster(actor))
          return instance()->calcEnemyPosition(actor);

        return Orig(actor);
      }
    };
    GetPlayerPosRs::InstallAtSymbol("_ZN2rs12getPlayerPosEPKN2al9LiveActorE");

    struct GetPlayerPos : Trampoline<GetPlayerPos> {
      static const sead::Vector3f& Callback(al::LiveActor* actor, int i) {
        if (instance()->isAMonster(actor))
          return instance()->calcEnemyPosition(actor);

        return Orig(actor, i);
      }
    };
    GetPlayerPos::InstallAtSymbol("_ZN2al12getPlayerPosEPKNS_9LiveActorEi");

    struct FindNearestPlayerId : Trampoline<FindNearestPlayerId> {
      static int Callback(al::LiveActor* actor, float maxDistance) {
        if (instance()->isAMonster(actor)) {
          return (al::getTrans(actor) - instance()->calcEnemyPosition(actor)).length() < maxDistance ? 0 : -1;
        }

        return Orig(actor, maxDistance);
      }
    };
    FindNearestPlayerId::InstallAtSymbol("_ZN2al19findNearestPlayerIdEPKNS_9LiveActorEf");

    struct TryFindNearestPlayerPos : Trampoline<TryFindNearestPlayerPos> {
      static bool Callback(sead::Vector3f* pos, const al::LiveActor* actor) {
        if (instance()->isAMonster(actor)) {
          *pos = instance()->calcEnemyPosition(actor);
          return true;
        }

        return Orig(pos, actor);
      }
    };
    TryFindNearestPlayerPos::InstallAtSymbol("_ZN2al23tryFindNearestPlayerPosEPN4sead7Vector3IfEEPKNS_9LiveActorE");

    struct FindNearestPlayerPos : Trampoline<FindNearestPlayerPos> {
      static const sead::Vector3f& Callback(al::LiveActor* actor) {
        if (instance()->isAMonster(actor))
          return instance()->calcEnemyPosition(actor);

        return Orig(actor);
      }
    };
    FindNearestPlayerPos::InstallAtSymbol("_ZN2al20findNearestPlayerPosEPKNS_9LiveActorE");
    struct GetPlayerHeadPos : Trampoline<GetPlayerHeadPos> {
      static const sead::Vector3f& Callback(al::LiveActor* actor) {
        if (instance()->isAMonster(actor))
          return instance()->calcEnemyPosition(actor);

        return Orig(actor);
      }
    };
    GetPlayerHeadPos::InstallAtSymbol("_ZN2rs16getPlayerHeadPosEPKN2al9LiveActorE");
    struct IsNearPlayerH : Trampoline<IsNearPlayerH> {
      static bool Callback(al::LiveActor* actor, f32 distance) {
        if (instance()->isAMonster(actor)) {
          return true;
          // return al::isNearH(actor, instance()->calcEnemyPosition(actor), distance);
        }

        return Orig(actor, distance);
      }
    };
    IsNearPlayerH::InstallAtSymbol("_ZN2rs13isNearPlayerHEPKN2al9LiveActorEf");

    StateCreationHook::InstallAtSymbol("_ZN2al5Scene18initSceneObjHolderEPNS_14SceneObjHolderE");
    StateUpdateHook::InstallAtSymbol("_ZN10StageScene7exePlayEv");
    ActorInit::InstallAtSymbol("_ZN2al12initActorSRTEPNS_9LiveActorERKNS_13ActorInitInfoE");
    ActorInitSRT::InstallAtSymbol("_ZN2al12initActorSRTEPNS_9LiveActorERKNS_13ActorInitInfoE");

    // disable revival patches
    patch::CodePatcher patcher(0xfdd44);
    patcher.WriteInst(inst::Movz(reg::W8, 0));
    patcher.Seek(0xfe078);
    patcher.WriteInst(inst::Movz(reg::W8, 1));

    patcher.Seek(0x514540); // world resource loader disable
    patcher.WriteInst(inst::Movz(reg::X0, 0));
    patcher.WriteInst(inst::Ret());
    patcher.Seek(0x514050);
    patcher.WriteInst(inst::Ret());

    patcher.Seek(0xa6b39c); // default scene resource heap size *3
    patcher.Write<u32>(0x52A25813);

    patcher.Seek("_ZN2al34sendMsgPushAndKillVelocityToTargetEPNS_9LiveActorEPNS_9HitSensorES3_", 0);
    ph::writeBooleanAndReturn(patcher, false);
    patcher.Seek("_ZN2al11sendMsgPushEPNS_9HitSensorES1_", 0);
    ph::writeBooleanAndReturn(patcher, false);

    // patcher.Seek("_ZN2al9ModelCtrl17setDitherAnimatorEPNS_14DitherAnimatorE", 0);
    // ph::writeReturn(patcher);

    struct EnemyAttackRewrite : Trampoline<EnemyAttackRewrite> {
      static bool Callback(al::HitSensor* sensor, al::HitSensor* other) {
        if (instance()->canAffect(al::getSensorHost(sensor), al::getSensorHost(other)))
          return mob::sendMsgEnemyAttack(sensor, other) || Orig(sensor, other);
        return Orig(sensor, other);
      }
    }; // namespace mob
    EnemyAttackRewrite::InstallAtSymbol("_ZN2al18sendMsgEnemyAttackEPNS_9HitSensorES1_");

    struct FileLoaderWacky : Trampoline<FileLoaderWacky> {
      static void* Callback(void* self, void* fileBase) {
        auto* fileName = getFunc<sead::SafeString*, void*>("_ZNK2al13FileEntryBase11getFileNameEv")(fileBase);
        Logger::log("filen aem %s\n", fileName->cstr());
        return Orig(self, fileBase);
      }
    };

    struct OhFuck : Trampoline<OhFuck> {
      static void Callback(al::LiveActor* actor) {
        if (instance()->startState > 1 || instance()->startState == 0 || !instance()->isAMonster(actor)) {
          return Orig(actor);
        }

        if (actor->mFlags->isDead || actor->mFlags->isClipped)
          return;

        // if (actor->mActorActionKeeper != nullptr) {
        // actor->mActorActionKeeper->updatePrev();
        // }
        if (actor->mModelKeeper != nullptr) {
          actor->mModelKeeper->update();
        }
      }
    };
    OhFuck::InstallAtSymbol("_ZN2al9LiveActor8movementEv");
    // FileLoaderWacky::InstallAtSymbol("_ZN2al16FileLoaderThread15requestLoadFileEPNS_13FileEntryBaseE");
    for (const auto& monster : MonsterInfo::getMonsters()) {
      if (monster.setup)
        monster.setup();
    }
  }

  void MonsterState::update() {
    player->mPlayerConst->mJumpGravity = par::get("JumpGravity", player->mPlayerConst->getJumpGravity());

    static int (*calculateDepth)(al::NerveKeeper*) = [](al::NerveKeeper* keeper) {
      if (!keeper)
        return 0;
      if (keeper->getStateCtrl() && keeper->getStateCtrl()->mCurrentState) {
        auto state = keeper->getStateCtrl()->mCurrentState->state;
        return calculateDepth(state->getNerveKeeper()) + 1;
      }
      return keeper->getCurrentNerve() != nullptr ? 1 : 0;
    };

    static void (*logNerveTree)(al::NerveKeeper*, int) = [](al::NerveKeeper* keeper, int leftOffset) {
      if (!keeper || !keeper->getCurrentNerve())
        return;

      Logger::log("\x1b[%dCNerve for %s: %s\n", leftOffset, getTypename(keeper->mParent),
                  getTypename(keeper->getCurrentNerve()));
      if (keeper->getStateCtrl() && keeper->getStateCtrl()->mCurrentState)
        logNerveTree(keeper->getStateCtrl()->mCurrentState->state->getNerveKeeper(), leftOffset);
    };

    if (par::get("DebugLogNerves", false)) {
      Logger::log("\x1b[200D\x1b[100B\x1b[%dA", calculateDepth(firstActor->getNerveKeeper()));
      logNerveTree(firstActor->getNerveKeeper(), 0);
    }

    if (firstActor && secondActor) {
      if (auto retarget = getActiveMonsterInfo(firstActor).retarget)
        firstTarget = retarget(firstActor);
      if (auto retarget = getActiveMonsterInfo(secondActor).retarget)
        secondTarget = retarget(secondActor);

      static const auto retargetSpawns = [](const al::LiveActor* actor, al::LiveActor* spawn, float distance = 100.0f) {
        auto enemy = instance()->getEnemyClean(actor);
        auto ownPos = al::getTrans(instance()->getEnemyClean(enemy));
        auto enemyPos = al::getTrans(enemy);
        auto dirToEnemy = enemyPos - ownPos + sead::Vector3f::ey * 150.0f;
        dirToEnemy.normalize();
        al::setTrans(spawn, ownPos + dirToEnemy * 100.0f);
      };

      retargetSpawns(firstActor, secondSpawn);
      retargetSpawns(secondActor, firstSpawn);

      if (par::clicked("Closer")) {
        static const auto closenActor = [](al::LiveActor* actor) {
          auto ownPos = al::getTrans(actor);
          auto enemy = instance()->getEnemyClean(actor);
          auto enemyPos = al::getTrans(enemy);
          auto dirToEnemy = enemyPos - ownPos;
          dirToEnemy.normalize();
          al::setTrans(actor, ownPos + dirToEnemy * 100.0f);
        };
        closenActor(getEnemyClean(secondActor));
        closenActor(getEnemyClean(firstActor));
      }
    }

    if (par::clicked("Crash")) {
      svcReturnFromException(0x1000);
    }

    if (par::clicked("Start")) {
      Logger::log("before start %d\n", startState);
      startState = 0;
      Logger::log("after start %d\n", startState);
    }

    if (startState > 1)
      startState--;

    if (!al::isActiveCamera(freecamTicket) && par::get("PlayerHidden", false)) {
      Logger::log("Player hidden\n");
      al::startCamera(player, freecamTicket, -1);
      Logger::log("Player hidden\n");
      player->startDemoPuppetable();
      Logger::log("Player hidden\n");
      player->mPlayerPuppet->hide();
      Logger::log("Player hidden\n");
      scene->mStageSceneLayout->end();
      Logger::log("Player hidden\n");
    }
    if (al::isActiveCamera(freecamTicket) && !par::get("PlayerHidden", false)) {
      Logger::log("Player unhidden\n");
      player->mPoseKeeper->mTrans.y += 1000.0f;
      Logger::log("Player unhidden\n");
      al::endCamera(player, freecamTicket, -1, false);
      Logger::log("Player unhidden\n");
      player->mPlayerPuppet->show();
      Logger::log("Player unhidden\n");
      player->endDemoPuppetable();
      Logger::log("Player unhidden\n");
      scene->mStageSceneLayout->start();
      Logger::log("Player unhidden\n");
    }

    if (al::isActiveCamera(freecamTicket) && al::isLessEqualStep(scene, 5)) {
      scene->mStageSceneLayout->end();
    }

    if (par::clicked("ReloadStage")) {
      PlayerHelper::warpPlayer(GameDataFunction::getCurrentStageName(player), player);
    }

    static const struct {
      const char* trigger;
      const char* stage;
    } stages[] = {
        {"ArenaBossRaidWorld", "ArenaBossRaidWorldExStage"},
        {"ArenaCapWorld", "ArenaCapWorldExStage"},
        {"ArenaCityWorld", "ArenaCityWorldExStage"},
        {"ArenaClashWorld", "ArenaClashWorldExStage"},
        {"ArenaCloudWorld", "ArenaCloudWorldExStage"},
        {"ArenaForestWorld", "ArenaForestWorldExStage"},
        {"ArenaLakeWorld", "ArenaLakeWorldExStage"},
        {"ArenaLavaWorld", "ArenaLavaWorldExStage"},
        {"ArenaMoonWorld", "ArenaMoonWorldExStage"},
        {"ArenaPeachWorld", "ArenaPeachWorldExStage"},
        {"ArenaSandWorld", "ArenaSandWorldExStage"},
        {"ArenaSeaWorld", "ArenaSeaWorldExStage"},
        {"ArenaSkyWorld", "ArenaSkyWorldExStage"},
        {"ArenaSnowWorld", "ArenaSnowWorldExStage"},
        {"ArenaWaterfallWorld", "ArenaWaterfallWorldExStage"},
    };

    for (auto [trigger, stage] : stages) {
      if (par::clicked(trigger)) {
        PlayerHelper::warpPlayer(stage, player);
      }
    }
  }

  void MonsterState::initAfterPlacementSceneObj(const al::ActorInitInfo& initInfo) {
    player = reinterpret_cast<PlayerActorHakoniwa*>(al::getPlayerActor(initInfo.mActorSceneInfo.mPlayerHolder, 0));
    freecam = alloc<PoserFreecam>("gex");
    freecamTicket = alCameraFunction::initCamera(freecam, player, initInfo, nullptr, 9);
    startState = 3;
    const auto group = initInfo.mAllActorsGroup;
    Logger::log("Group: %s\n", group->mGroupName);

    firstOffspring.clear();
    secondOffspring.clear();

    if (firstSpawn && secondSpawn) {
      Logger::log("nice! found both");
    } else {
      Logger::log("oh my god you didn't find both actors raaaaah\n");
      svcSleepThread(1e10);
      // svcReturnFromException(0);
      return;
    }

    auto max = static_cast<int>(MonsterInfo::getMonsters().size()) - 1;
    firstMonster = std::clamp(par::get("FirstMonster", 0), 0, max);
    secondMonster = std::clamp(par::get("SecondMonster", 0), 0, max);
    if (par::get("FlipBit", false) && sead::Random().getBool()) {
      std::swap(firstMonster, secondMonster);
    }

    spawnMonster(firstActor, getActiveMonsterInfo(true), initInfo);
    al::copyPose(firstActor, firstSpawn);
    spawnMonster(secondActor, getActiveMonsterInfo(false), initInfo);
    al::copyPose(secondActor, secondSpawn);
  }

  void MonsterState::spawnMonster(al::LiveActor*& actor, const MonsterInfo& info, const al::ActorInitInfo& initInfo) {
    const al::StringTmp<64> name("PlacementData/%s.byml", info.gameName);
    auto placementInfo = al::PlacementInfo();
    const auto file = al::loadFile(name, 8);
    Logger::log("File %s %p\n", name.cstr(), file);
    placementInfo.mPlacementIter = al::ByamlIter(file);
    al::ActorInitInfo actorInitInfo;
    actorInitInfo.initNoViewId(&placementInfo, initInfo);
    const char *objectName = nullptr, *className = nullptr, *displayName = nullptr;
    al::getObjectName(&objectName, actorInitInfo);
    al::getClassName(&className, actorInitInfo);
    al::tryGetDisplayName(&displayName, actorInitInfo);
    Logger::log("Class name %s, Object name %s, display name %s\n", className, objectName, displayName);

    actor = initInfo.mActorFactory->getCreator(className)(displayName ? displayName : objectName);
    currentlyInitializing = actor;
    auto position = calcPosition(actor);
    if (info.start)
      info.start(actor, actorInitInfo, position);

    al::initCreateActorWithPlacementInfo(actor, actorInitInfo, placementInfo);
    if (info.started)
      info.started(actor, actorInitInfo, position);

    const auto pos = al::getTrans(actor);
    Logger::log("Spawned %s at %.2f %.2f %.2f %s\n", info.gameName, pos.x, pos.y, pos.z, BTOC(al::isAlive(actor)));
    if (al::isDead(actor))
      actor->appear();
    currentlyInitializing = nullptr;
}
} // namespace mob