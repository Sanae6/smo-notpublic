#include "CaptureState.hpp"
#include "CaptureInfo.hpp"
#include "al/Library/Base/String.h"
#include "al/Library/File/FileLoader.h"
#include "al/Library/File/FileUtil.h"
#include "al/Library/Layout/LayoutActor.h"
#include "al/Library/Layout/LayoutActorUtil.h"
#include "al/Library/LiveActor/ActorFlagFunction.h"
#include "al/Library/LiveActor/ActorMovementFunction.h"
#include "al/Library/LiveActor/ActorPoseKeeper.h"
#include "al/Library/LiveActor/ActorSensorFunction.h"
#include "al/Library/LiveActor/LiveActor.h"
#include "al/Library/Memory/HeapUtil.h"
#include "al/Library/Nerve/Nerve.h"
#include "al/Library/Nerve/NerveKeeper.h"
#include "al/Library/Nerve/NerveStateBase.h"
#include "al/Library/Nerve/NerveUtil.h"
#include "al/Library/Placement/PlacementFunction.h"
#include "al/Library/Placement/PlacementInfo.h"
#include "al/Library/Player/PlayerHolder.h"
#include "al/Library/Resource/ResourceHolder.h"
#include "al/Library/Scene/Scene.h"
#include "al/Library/System/SystemKit.h"
#include "al/Library/Yaml/ByamlIter.h"
#include "al/Library/Yaml/ByamlUtil.h"
#include "al/Project/ArchiveEntry.h"
#include "al/Project/ArchiveHolder.h"
#include "armv8/instructions/op100x/move_wide_immediate/movz.hpp"
#include "armv8/instructions/opx101/logical_shifted_register/mov_register.hpp"
#include "armv8/register.hpp"
#include "diag/assert.hpp"
#include "fs/fs_types.hpp"
#include "game/GameData/GameDataHolderAccessor.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "game/Player/States/PlayerStateHack.h"
#include "game/Scene/ProjectActorFactory.h"
#include "logger/Logger.hpp"
#include "logger/Params.h"
#include "prim/seadSafeString.h"
#include "rs/util/SensorUtil.h"
#include "utils/ForwardDecls.hpp"
#include "utils/Helpers.h"
#include <al/Library/SaveData/SaveDataFunction.h>
#include <fs/fs_files.hpp>
#include <game/GameData/GameDataFunction.h>
#include <game/HakoniwaSequence/HakoniwaSequence.h>
#include <game/StageScene/StageScene.h>
#include <game/System/Application.h>
#include <game/System/GameSystem.h>
#include <gfx/seadPrimitiveRenderer.h>
#include <helpers/PlayerHelper.h>
#include <stream/seadRamStream.h>

class Pukupuku {};
namespace cs {
  SEAD_SINGLETON_DISPOSER_IMPL(CaptureState)
  void CaptureState::init() {
    struct StateCreationHook : public Trampoline<StateCreationHook> {
      static al::SceneObjHolder* Callback(al::Scene* scene, al::SceneObjHolder* holder) {
        Orig(scene, holder);
        CaptureState::instance()->loadingSave = false;
        if (isSameType<StageScene>(scene))
          holder->setSceneObj(CaptureState::instance(), 0x40);
        return holder;
      }
    };

    struct StateUpdateHook : public Trampoline<StateUpdateHook> {
      static void Callback(StageScene* scene) {
        instance()->update();
        Orig(scene);
      }
    };

    struct ReviveScreenPositioner : public Trampoline<ReviveScreenPositioner> {
      struct EnemyStateReviveInsideScreen : public al::ActorStateBase {
        sead::Quatf quat;
        sead::Vector3f pos;
        bool bools[3];
      };
      static void Callback(EnemyStateReviveInsideScreen* state, al::LiveActor* actor) {
        Orig(state, actor);
        if (actor == instance()->capture)
          state->pos = instance()->calcPosition();
      }
    };

    struct ResetPositioner : public Trampoline<ResetPositioner> {
      struct EnemyStateReset : public al::ActorStateBase {
        sead::Vector3f pos;
        sead::Vector3f rotate;
        f32 distanceFromPlayer;
        bool isRevive;
        bool shouldRevalidateHitSensors;
        class EnemyCap* enemyCap;
      };
      static void Callback(EnemyStateReset* state, al::LiveActor* actor, al::ActorInitInfo const& initInfo,
                           EnemyCap* cap) {
        Orig(state, actor, initInfo, cap);
        if (actor == instance()->capture)
          state->pos = instance()->calcPosition();
      }
    };

    struct ActorInit : Trampoline<ActorInit> {
      static void Callback(al::LiveActor* actor, const al::ActorInitInfo& initInfo) {
        Orig(actor, initInfo);

        if (actor == instance()->capture) {
          al::setTrans(actor, instance()->calcPosition());
        }
      }
    };

    struct SaveOnGo : public Trampoline<SaveOnGo> {
      static void Callback(GameProgressData* data, s32 index) {
        auto checkIfCap = []() { // cap is already marked as visited on a new save
          nn::fs::FileHandle handle{};

          if (R_FAILED(nn::fs::OpenFile(&handle, "sd:/kingdom_save_0.save", nn::fs::OpenMode_Read)))
            return true;

          nn::fs::CloseFile(handle);
          return false;
        };
        Logger::log("first time :3 %d\n", index, data->mIsFirstTimeGoWorld[index]);
        if (data->mIsFirstTimeGoWorld[index] || (index == 0 && checkIfCap())) {
          auto sequence = (HakoniwaSequence*)GameSystemFunction::getGameSystem()->mCurSequence;
          al::StringTmp<32> fileName("%s%d.bin", "File", sequence->mGameDataHolder.mData->mGameFileIndex + 1);
          Logger::log("Are we going back? %d\n", sequence->mGameDataHolder.mData->mDataFile->mIsPlayDemoOpening);
          sequence->mGameDataHolder.mData->mDataFile->mIsPlayDemoOpening = false;
          sequence->mGameDataHolder.mData->writeToSaveDataBuffer(fileName.cstr());

          al::StringTmp<128> path("sd:/kingdom_save_%d.save", index);
          nn::fs::CreateFile(path.cstr(), 0x200000);
          nn::fs::FileHandle handle{};
          nn::fs::OpenFile(&handle, path.cstr(), nn::fs::OpenMode_Write);
          nn::fs::WriteFile(handle, 0, al::getSaveDataWorkBuffer(), 0x200000,
                            nn::fs::WriteOption{nn::fs::WriteOptionFlag_Flush});
          nn::fs::CloseFile(handle);
          Logger::log("saved first time to %s\n", path.cstr());
        }

        Orig(data, index);
      }
    };

    struct AnagramModelName : public Trampoline<AnagramModelName> {
      static void Callback(al::LiveActor* actor, al::ActorInitInfo* initInfo, sead::SafeString* objectFolder,
                           sead::SafeString* objectName, char* suffix) {
        if (actor == instance()->capture && isAnagramAlphabetCharacter())
          *objectName = instance()->capture->getName();
        Orig(actor, initInfo, objectFolder, objectName, suffix);
      }
    };

    struct AnagramAlphabetTestBase : public Trampoline<AnagramAlphabetTestBase> {
      static void Callback(AnagramAlphabet* alphabet, AnagramAlphabetCharacter* character) {
        if (alphabet)
          Orig(alphabet, character);
      }
    };

    struct AnagramAlphabetTestComplete : public Trampoline<AnagramAlphabetTestComplete> {
      static void Callback(AnagramAlphabet* alphabet) {
        if (alphabet)
          Orig(alphabet);
      }
    };

    struct AnagramAlphabetTestEndHack : public Trampoline<AnagramAlphabetTestEndHack> {
      static void Callback(AnagramAlphabet* alphabet) {
        if (alphabet)
          Orig(alphabet);
      }
    };
    struct NerveLogging : public Trampoline<NerveLogging> {
      static void Callback(al::NerveKeeper* keeper) {
        static auto checkLog = [](al::NerveKeeper* keeper) {
          if (isSameType<HakoniwaSequence>(keeper->mParent)) {
            if (keeper->mNextNerve) {
              if (keeper->mCurrentNerve)
                Logger::log("Coming from %s, going to nerve %s\n", getTypename(keeper->mCurrentNerve),
                            getTypename(keeper->mNextNerve));
              else
                Logger::log("Starting, going to nerve %s\n", getTypename(keeper->mNextNerve));
            }
          };
        };

        checkLog(keeper);
        keeper->tryChangeNerve();
        keeper->mCurrentNerve->execute(keeper);
        keeper->mStep++;
        checkLog(keeper);
        keeper->tryChangeNerve();
      }
    };

    struct HijackPlayerStartPosition : Trampoline<HijackPlayerStartPosition> {
      static void Callback(PlayerActorHakoniwa* player, const al::ActorInitInfo& info,
                           const PlayerInitInfo& playerInfo) {
        SavedPlayerLocation& savedLocation = instance()->lastPlayerLocation;
        if (savedLocation.isSaved) {
          savedLocation.isSaved = false;
          playerInfo.position = savedLocation.position;
          playerInfo.rotation = savedLocation.rotation;
          Logger::log("loaded saved player location!\n");
        } else {
          Logger::log("normal player spawn\n");
        }
        Orig(player, info, playerInfo);
      }
    }; // PlayerActorHakoniwa::initPlayer

    for (auto& capture : CaptureInfo::getCaptures()) {
      if (capture.setup)
        capture.setup();
    }

    static bool (*hackListIsDebug)(const al::ByamlIter&, const char*) = [](const al::ByamlIter& iter, const char* key) {
      const char* hackName = al::getByamlKeyString(iter, "HackName");
      const auto& infos = CaptureInfo::getCaptures();
      for (int i = 0; i < infos.size(); i++) {
        const CaptureInfo& info = infos[i];
        if (al::isEqualString(hackName, info.hackName ? info.hackName : info.gameName)) {
          return instance()->capturesChosen[i];
        }
      }
      return true;
    };

    // lyl crafty
    patch::CodePatcher p(0x1F365C);
    HijackPlayerStartPosition::InstallAtSymbol(
        "_ZN19PlayerActorHakoniwa10initPlayerERKN2al13ActorInitInfoERK14PlayerInitInfo");
    if (!par::get("MoonGet", true)) {
      p.Seek(0x1cd878);
      ph::pretendBoolean(p, false); // disable moon grabbing
    }

    // hack list modding
    p.Seek(0x1ecd04);
    p.BranchLinkInst((void*)hackListIsDebug); // disable non-supported captures
    p.Seek(0x1ece78);
    p.BranchLinkInst((void*)hackListIsDebug); // same
    p.Seek("_ZN16GameDataFunction23isExistInHackDictionaryE22GameDataHolderAccessorPKc", 0x00);
    ph::writeBooleanAndReturn(p, true);
    p.Seek(0x1ecc5c);
    static char16_t* (*hackListFooterMessage)(al::IUseMessageSystem const*, char const*, char const*) =
        [](al::IUseMessageSystem const* a, char const* b, char const* c) {
          Logger::log("stupid: %s\n", c);
          return al::getSystemMessageString(a, b, "MenuMessage_Footer");
        };
    p.BranchLinkInst((void*)hackListFooterMessage);
    p.Seek(0x4dba80);
    p.WriteInst(inst::Movz(reg::X1, 2)); // increase states
    p.Seek(0x4ca95c);
    p.WriteInst(inst::Movz(reg::X0, 0x130));
    static struct CollectionListShineNerve : public al::Nerve {
      void execute(al::NerveKeeper* keeper) const override {
        StageSceneStateGetShine* state = reinterpret_cast<StageSceneStateGetShine*>(keeper->mParent);
        if (al::isFirstStep(keeper->mParent)) {
          auto* ptr = unsafeRef<StageSceneStateCollectionList*>(state, 0x128);
          Logger::log("first state frame %p!\n", ptr);
          setNerveOffset(reinterpret_cast<al::IUseNerve*>(ptr), 0x1d8d730);
        }
        al::updateNerveStateAndNextNerve(keeper->mParent, nerveAt(0x1d8dc50));
      }
    } CollectionList;
    struct SetupCollectionStateInGet : Trampoline<SetupCollectionStateInGet> {
      static void Callback(StageScene* scene, const al::ActorInitInfo& initInfo) {
        Orig(scene, initInfo);
        if (isSameType<StageScene>(scene)) {
          Logger::log("prepping get shine state\n");
          unsafeRef<StageSceneStateCollectionList*>(scene->mStateGetShine, 0x128) = scene->mStateCollectionList;
          al::addNerveState(reinterpret_cast<al::IUseNerve*>(scene->mStateGetShine),
                            reinterpret_cast<al::NerveStateBase*>(scene->mStateCollectionList), &CollectionList,
                            "CollectionList :3");
        }
      }
    };
    SetupCollectionStateInGet::InstallAtSymbol("_ZN2al5Scene7endInitERKNS_13ActorInitInfoE");
    static void (*nerveFixer)(al::IUseNerve* user, const al::Nerve* nerve) = [](al::IUseNerve* user,
                                                                                const al::Nerve* nerve) {
      Logger::log("fixer upper: %s %s, %d\n", getTypename(user->getNerveKeeper()->getCurrentNerve()),
                  getTypename(nerve), nerve == nerveAt(0x1d8dc50));
      if (nerve == nerveAt(0x1d8dc50)) {
        StageSceneStateGetShine* state = reinterpret_cast<StageSceneStateGetShine*>(user->getNerveKeeper()->mParent);
        StageSceneStateCollectionList* ptr = unsafeRef<StageSceneStateCollectionList*>(state, 0x128);
        unsafeRef<s32>(ptr, 0x80) = false;
        unsafeRef<bool>(ptr, 0x84) = false;
        unsafeRef<bool>(ptr, 0x91) = false;
        unsafeRef<bool>(ptr, 0x95) = true;
        al::setNerve(user, &CollectionList);
        unsafeRef<al::LayoutActor*>(ptr, 0x28)->appear();
      } else
        al::setNerve(user, nerve);
    };
    p.Seek(0x4dc6f8);
    p.BranchLinkInst((void*)nerveFixer);
    p.Seek(0x4dcc10);
    p.BranchInst((void*)nerveFixer);

    p.Seek(0x4da9c4);
    p.WriteInst(inst::MovRegister(reg::X0, reg::X19));
    static bool (*shouldClose)(StageSceneStateCollectionList*) = [](StageSceneStateCollectionList* state) {
      if (rs::isTriggerUiDecide(unsafeRef<al::Scene*>(state, 0x18))) {
        struct HackPartsData {
          sead::FixedSafeString<64>* name;
          bool hasBeenCaptured;
        };
        void* hackListLayout = unsafeRef<void*>(state, 0x28);
        HackPartsData* hackPartsData = unsafeRef<HackPartsData*>(hackListLayout, 0x138);
        struct CommonHorizontalList* list = unsafeRef<CommonHorizontalList*>(hackListLayout, 0x130);
        s32 index = unsafeRef<s32>(list, 0x3c);
        auto captureInfos = CaptureInfo::getCaptures();
        for (int j = 0; j < captureInfos.size(); j++) {
          auto& info = captureInfos[j];
          if (al::isEqualString(hackPartsData[index].name->cstr(), info.hackName ? info.hackName : info.gameName)) {
            instance()->chosenCapture = j;
            instance()->capturesChosen[j] = true;
            Logger::log("savering %s %d\n", hackPartsData[index].name->cstr(), j);
            SavedPlayerLocation& savedLocation = instance()->lastPlayerLocation;
            savedLocation.isSaved = true;
            savedLocation.position = al::getTrans(instance()->player);
            savedLocation.rotation = al::getQuat(instance()->player);
            PlayerHelper::warpPlayer(GameDataFunction::getCurrentStageName(instance()->player), instance()->player);
            instance()->saveState();
            return true;
          }
        }
      }
      return false;
    };
    p.BranchLinkInst((void*)shouldClose);
    p.Seek(0x4daa04);
    ph::pretendBoolean(p, false);
    p.Seek(0x4daa70);
    ph::pretendBoolean(p, false);

    struct ResourceCategoryOverrider : Trampoline<ResourceCategoryOverrider> {
      static const char* Callback(al::ResourceSystem* system, const sead::SafeString& resource) {
        if (instance() && instance()->overrideResourceCategory && !resource.startsWith("SoundData/")) {
          Logger::log("overriding resource category %s\n", resource.cstr());
          return "Captures";
        }
        return Orig(system, resource);
      }
    };
    ResourceCategoryOverrider::InstallAtSymbol(
        "_ZNK2al14ResourceSystem25findCategoryNameFromTableERKN4sead14SafeStringBaseIcEE");
    struct DestroySceneHeapResource : Trampoline<DestroySceneHeapResource> {
      static void Callback(bool unloadResources) {
        Logger::log("removing capture category\n");
        al::ResourceSystem* system = alProjectInterface::getSystemKit()->getResourceSystem();
        for (int i = 0; i < system->count; i++) {
          al::ResourceCategory* cat = system->categories[i];
          if (al::isEqualString(cat->name, "Captures")) {
            Logger::log("Clearing all archives for Captures resource cat\n");
            cat->aa.forEach([](const sead::SafeString& key, al::Resource* resource) {
              al::ArchiveHolder* holder = alProjectInterface::getSystemKit()->getFileLoader()->mArchiveHolder;
              al::StringTmp<152> keyFilename("%s.szs", key.cstr());

              al::ArchiveEntry* entry = holder->tryFindEntry(keyFilename);
              if (!entry)
                EXL_ABORT(0, "entry failed to find %s", keyFilename.cstr());

              Logger::log("cleared archive for %s\n", keyFilename);
              entry->clear();
            });
          }
        }
        al::removeResourceCategory("Captures");
        Orig(unloadResources);
      }
    };
    DestroySceneHeapResource::InstallAtSymbol("_ZN2al16destroySceneHeapEb");
    struct KillRollParts : Trampoline<KillRollParts> {
      static void Callback(StageSceneStateCollectionList* list) {
        auto* layout = unsafeRef<al::LayoutActor*>(list, 0x70);
        al::hidePane(layout, "TxtPage");
        al::hidePane(layout, "GuideLR");
        auto* hackLayout = unsafeRef<al::LayoutActor*>(list, 0x28);
        al::hidePaneRoot(unsafeRef<al::LayoutActor*>(hackLayout, 0x180));
        Orig(list);
      }
    };
    KillRollParts::InstallAtSymbol("_ZN29StageSceneStateCollectionList15exeViewHackListEv");

    // forces isExistHome to always return true (specifically for MapLayout, so it shows the odyssey warp point)
    //    p.Seek(0x1F365C);
    //    p.WriteInst(inst::Movz(reg::W0, 1));

    // forces the odyssey to always fully init, even if the current kingdom is not unlocked
    //    p.Seek(0x309904); // ShineTowerRocket::init + 0xAA4
    //    p.WriteInst(inst::Movz(reg::W0, 1));

    // removes a check for the Odyssey to stop init if cascade is not unlocked
    //    p.Seek(0x309938); // ShineTowerRocket::init + 0xAD8
    //    p.WriteInst(inst::Movz(reg::W0, 1));

    // world map unlock all worlds
    p.Seek(0x52a0c8);
    //    p.WriteInst(inst::Movz(reg::X0, 1));
    //    p.WriteInst(inst::Ret());

    // make lost globing work fine
    //    p.Seek(0x534ec0);
    //    p.WriteInst(inst::CmpImmediate(reg::W8, 4));
    //    p.Seek(0x534ec8);
    //    p.WriteInst(inst::Movz(reg::W8, 5));

    // make ruined globing work fine
    //    p.Seek(0x534f00);
    //    p.WriteInst(inst::CmpImmediate(reg::W8, 6));
    //    p.Seek(0x534f08);
    //    p.WriteInst(inst::Movz(reg::W8, 7));

    // make cloud globing work fine
    //    p.Seek(0x534ea0);
    //    p.WriteInst(inst::Movz(reg::W8, 5));
    //    p.Seek(0x30c398);
    //    p.WriteInst(inst::Movz(reg::W8, 5));
    //    p.Seek(0x30f5dc);
    //    p.WriteInst(inst::Movz(reg::X0, 0));

    //    static bool (*isCrashHome)(GameDataHolderAccessor holder) = [](GameDataHolderAccessor holder) {
    //      auto progress = holder.mData->mDataFile->mGameProgressData;
    //      if (progress->mHomeStatus == HomeShipStates::CRASH)
    //        progress->mHomeStatus = HomeShipStates::REPAIR;
    //      if (progress->mHomeStatus == HomeShipStates::CRASHRUINED)
    //        progress->mHomeStatus = HomeShipStates::REPAIRRUINED;
    //
    //      return false;
    //    };
    //    p.Seek(0x30da30);
    //    p.BranchLinkInst((void*)isCrashHome);

    // disable home status correction in GameProgressData::checkAndChangeCorrectStatus
    p.Seek(0x534df0);
    p.WriteInst(inst::Nop());
    p.Seek(0x534e10);
    p.WriteInst(inst::Nop());

    // entirely disable the world resource loader for home stage resources
    p.Seek(0x514540);
    p.WriteInst(inst::Movz(reg::X0, 0));
    p.WriteInst(inst::Ret());
    p.Seek(0x514050);
    p.WriteInst(inst::Ret());

    StateCreationHook::InstallAtSymbol("_ZN2al5Scene18initSceneObjHolderEPNS_14SceneObjHolderE");
    StateUpdateHook::InstallAtSymbol("_ZN10StageScene7exePlayEv");
    ActorInit::InstallAtSymbol("_ZN2al12initActorSRTEPNS_9LiveActorERKNS_13ActorInitInfoE");
    SaveOnGo::InstallAtSymbol("_ZN16GameProgressData17setAlreadyGoWorldEi");
    ReviveScreenPositioner::InstallAtSymbol("_ZN28EnemyStateReviveInsideScreenC1EPN2al9LiveActorE");
    ResetPositioner::InstallAtSymbol("_ZN15EnemyStateResetC1EPN2al9LiveActorERKNS0_13ActorInitInfoEP8EnemyCap");
    AnagramModelName::InstallAtOffset(0x8dc12c);
    AnagramAlphabetTestBase::InstallAtSymbol("_ZN15AnagramAlphabet8testBaseEP24AnagramAlphabetCharacter");
    AnagramAlphabetTestComplete::InstallAtSymbol("_ZN15AnagramAlphabet12testCompleteEv");
    AnagramAlphabetTestEndHack::InstallAtSymbol("_ZN15AnagramAlphabet11testEndHackEv");
    NerveLogging::InstallAtSymbol("_ZN2al11NerveKeeper6updateEv");
    static bool (*loadData)(void*) = [](void* pauseMenuState) {
      return CaptureState::instance()->isLoadingSave() ||
             unsafeRef<bool>(unsafeRef<void*>(pauseMenuState, 0x70), 0x180);
    };
    p.Seek("_ZNK24StageSceneStatePauseMenu10isLoadDataEv", 0);
    p.BranchInst((void*)loadData);
    static bool (*isAlreadyDone)(GameDataHolder*) = [](GameDataHolder* holder) {
      auto func = getFunc<bool, SaveDataAccessSequence*>("_ZNK22SaveDataAccessSequence10isDoneSaveEv");
      return CaptureState::instance()->isLoadingSave() || func(holder->mSaveDataAccessSequence);
    };
    p.Seek("_ZN22SaveDataAccessFunction10isDoneSaveEP14GameDataHolder", 0);
    p.BranchInst((void*)isAlreadyDone);
    static bool (*isAlreadyPlayedOpening)(GameDataHolderAccessor) = [](GameDataHolderAccessor accessor) {
      return !CaptureState::instance()->isLoadingSave() && accessor.mData->mDataFile->mIsPlayDemoOpening;
    };
    p.Seek("_ZN16GameDataFunction17isPlayDemoOpeningE22GameDataHolderAccessor", 0);
    p.BranchInst((void*)isAlreadyPlayedOpening);
    static bool (*isAlreadyDoneLoadinWorldResource)(WorldResourceLoader*) = [](WorldResourceLoader* loader) {
      return CaptureState::instance()->isLoadingSave() || loader->mWorldResourceLoader->isDone();
    };
    p.Seek("_ZNK19WorldResourceLoader22isEndLoadWorldResourceEv", 0);
    p.BranchInst((void*)isAlreadyDoneLoadinWorldResource);
    struct WorldResourceUnloader : Trampoline<WorldResourceUnloader> {
      static float Callback(WorldResourceLoader* loader) {
        //        if (CaptureState::instance()->isLoadingSave() || loader->mMaxLoadCount <= loader->mCurLoadCount)
        return 101.f;

        return ((float)loader->mCurLoadCount * 100.0f) / (float)loader->mMaxLoadCount;
      }
    };
    WorldResourceUnloader::InstallAtSymbol("_ZNK19WorldResourceLoader15calcLoadPercentEv");
    {
      auto holder = ((HakoniwaSequence*)GameSystemFunction::getGameSystem()->mCurSequence)->mGameDataHolder.mData;
      struct NoMoreStageLocks : public Replace<NoMoreStageLocks> {
        static int Callback() { return 0; }
      };

      //      NoMoreStageLocks::InstallAtSymbol("_ZNK14GameDataHolder18findUnlockShineNumEPbi");
    }
  }
  void CaptureState::loadState() {
    nn::fs::FileHandle handle{};
    auto res = nn::fs::OpenFile(&handle, stateFilePath, nn::fs::OpenMode_Read);
    if (R_FAILED(res)) {
      Logger::log("state not yet...\n");
      return;
    }

    nn::fs::ReadFile(handle, 0, &chosenCapture, sizeof(chosenCapture));
    nn::fs::ReadFile(handle, sizeof(chosenCapture), capturesChosen.data(), sizeof(capturesChosen));
    nn::fs::CloseFile(handle);
  }
  void CaptureState::saveState() {
    bool set = true;
    for (int i = 0; i < capturesChosen.size(); i++) {
      if (!capturesChosen[i])
        set = false;
    }
    if (set) {
      capturesChosen = {};
    }
    nn::fs::FileHandle handle{};
    Logger::log("wrigning state %s\n", stateFilePath);
    nn::fs::DeleteFile(stateFilePath);
    nn::fs::CreateFile(stateFilePath, 64);
    nn::fs::OpenFile(&handle, stateFilePath, nn::fs::OpenMode_Write);
    nn::fs::WriteFile(handle, 0, &chosenCapture, sizeof(chosenCapture), nn::fs::WriteOption{0});
    nn::fs::WriteFile(handle, sizeof(chosenCapture), capturesChosen.data(), sizeof(capturesChosen),
                      nn::fs::WriteOption{nn::fs::WriteOptionFlag_Flush});
    nn::fs::CloseFile(handle);
  }
  void CaptureState::initAfterPlacementSceneObj(const al::ActorInitInfo& initInfo) {
    player = reinterpret_cast<PlayerActorHakoniwa*>(al::getPlayerActor(initInfo.mActorSceneInfo.mPlayerHolder, 0));
    ((GameDataHolder*)initInfo.mActorSceneInfo.mSceneObjHolder->getObj(18))->mDataFile->mIsEnableCap = true;
    player->mHackCap->hide(false);
    player->mPlayerAnimator->forceCapOn();

    instance()->overrideResourceCategory = true;
    al::addResourceCategory("Captures", 64, al::getSceneHeap());

    auto& captureInfo = getActiveCaptureInfo();
    Logger::log("Creating capture %s\n", captureInfo.gameName);
    if (isAnagramAlphabetCharacter()) {
      const char* choices[] = {
          "AnagramAlphabetM", "AnagramAlphabetA", "AnagramAlphabetR", "AnagramAlphabetI", "AnagramAlphabetO",
      };
      capture = alloc<AnagramAlphabetCharacter>(choices[sead::Random().getU32(5)]);
    } else
      capture = initInfo.mActorFactory->getCreator(captureInfo.gameName)(captureInfo.gameName);
    al::StringTmp<64> name("PlacementData/%s.byml", captureInfo.gameName);
    auto placementInfo = al::PlacementInfo();
    auto file = al::loadFile(name, 8);
    Logger::log("File %s %p\n", name.cstr(), file);
    placementInfo.mPlacementIter = al::ByamlIter(file);
    al::ActorInitInfo actorInitInfo;
    actorInitInfo.initNoViewId(&placementInfo, initInfo);
    const char* namePtr = nullptr;
    al::tryGetObjectName(&namePtr, actorInitInfo);
    Logger::log("Object name %s\n", namePtr);
    if (captureInfo.hackStart)
      captureInfo.hackStart(capture, instance()->calcPosition());

    al::initCreateActorWithPlacementInfo(capture, actorInitInfo, placementInfo);
    if (captureInfo.hackStarted)
      captureInfo.hackStarted(capture);

    instance()->overrideResourceCategory = false;
    Logger::log("Created %s to be magically captured\n", captureInfo.gameName);
  }
  void CaptureState::update() {
    if (par::clicked("ReloadStage")) {
      PlayerHelper::warpPlayer(GameDataFunction::getCurrentStageName(player), player);
    }

    if (par::clicked("WipeSave")) {
      sead::FixedSafeString<128> stageName(GameDataFunction::getCurrentStageName(player));
      auto worldId = GameDataFunction::getCurrentWorldId(player);
      auto* holder = GameDataHolderAccessor(player).mData;
      auto* file = GameDataFunction::getGameDataFile(GameDataHolderAccessor(player));
      file->initializeData();
      file->mIsPlayDemoOpening = true;
      file->mStageNameCurrent = stageName;
      file->mCurWorldId = worldId;
      holder->setRequireSave();
      PlayerHelper::warpPlayer(stageName.cstr(), player);
      Logger::log("Wiped save\n");
    }

    if (par::clicked("ResetKingdom")) {
      if (!al::isDoneSaveDataSequence()) {
        Logger::log("Saving isn't done...\n");
        return;
      }
      int index = GameDataFunction::getCurrentWorldId(player);
      auto gameDataHolder = GameDataHolderAccessor(player).mData;
      al::StringTmp<128> path("sd:/kingdom_save_%d.save", index);
      nn::fs::FileHandle handle{};
      Logger::log("Reading %s\n", path.cstr());
      auto res = nn::fs::OpenFile(&handle, path.cstr(), nn::fs::OpenMode_Read);
      if (R_FAILED(res)) {
        Logger::log("FAILED TO READ %s: %d\n", path.cstr(), res.value);
        return;
      }
      s32 fileIndex = (s32)gameDataHolder->mGameFileIndex;
      al::StringTmp<32> fileName("File%d.bin", fileIndex + 1);
      nn::fs::ReadFile(handle, 0, al::getSaveDataWorkBuffer(), 0x200000);
      nn::fs::CloseFile(handle);
      Logger::log("Loaded file into save data work buffer\n");
      auto file = gameDataHolder->getGameDataFile(fileIndex);
      Logger::log("Got file: %p\n", file);
      al::writeSaveDataSync(fileName.cstr(), 0x200000, 1);
      Logger::log("Wrote file %s\n", fileName.cstr());
      file->initializeData();
      Logger::log("Creating ram\n");
      sead::RamReadStream stream(al::getSaveDataWorkBuffer(), 0x200000, nullptr);
      stream.setMode(sead::Stream::Modes::Binary);
      Logger::log("Reading from stream\n");
      if (!file->readFromStream(&stream, gameDataHolder->mSaveFileBuffer)) {
        Logger::log("failed to read\n");
        file->initializeData();
      }
      Logger::log("Done\n");
      gameDataHolder->mNextDataFile = file;
      file->mIsPlayDemoOpening = true;
      loadingSave = true;

      auto stageScene = ((HakoniwaSequence*)GameSystemFunction::getGameSystem()->mCurSequence)->mStageScene;
      unsafeRef<bool>(stageScene->mStagePauseMenu, 0xc0) = false;
      stageScene->kill();
      Logger::log("killed current scene\n");
    }

    if (par::clicked("UnlockNextKingdom")) {
      GameDataFunction::unlockWorld(GameDataHolderAccessor(player), GameDataFunction::getNextWorldId(player));
    }

    if (par::clicked("TeleportCaptureHere")) {
      al::setTrans(capture, instance()->calcPosition());
      if (capture->getFlags()->isDead)
        capture->appear(); // try to bring it back, if this fails then give up lol
    }

    //    if (isSameType<Pukupuku>(capture))
    //      unsafeRef<int>(capture, 0x150) = 0;

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
      Logger::log("\x1b[200D\x1b[100B\x1b[%dA", calculateDepth(capture->getNerveKeeper()));
      logNerveTree(capture->getNerveKeeper(), 0);
      Logger::log("\x1b[100B\x1b[100C\x1b[%dA", calculateDepth(capture->getNerveKeeper()));
      logNerveTree(unsafeRef<al::LiveActor*>(unsafeRef<al::NerveStateBase*>(capture, 0x168), 0x38)->getNerveKeeper(),
                   0);
    }

    lastCapturePos = al::getTrans(capture);
  }
  void CaptureState::draw(sead::TextWriter* writer) {
    writer->printf("\n\nLast capture position %.02f %.02f %.02f\n", lastCapturePos.x, lastCapturePos.y,
                   lastCapturePos.z);
  }
} // namespace cs
