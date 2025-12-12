#include "ExceptionHandler.h"
#include "al/Library/Memory/HeapUtil.h"
#include "fs.h"
#include "helpers/InputHelper.h"
#include "helpers/PlayerHelper.h"
#include "imgui_backend/imgui_impl_nvn.hpp"
#include "imgui_nvn.h"
#include "lib.hpp"
#include "logger/Logger.hpp"
#include "patches.hpp"

#include <basis/seadRawPrint.h>
#include <devenv/seadDebugFontMgrNvn.h>
#include <gfx/seadTextWriter.h>
#include <gfx/seadViewport.h>
#include <heap/seadHeapMgr.h>
#include <prim/seadSafeString.h>

#include <al/Library/File/FileLoader.h>
#include <al/Library/File/FileUtil.h>

#include <capture/CaptureState.hpp>
#include <game/GameData/GameDataFunction.h>
#include <game/HakoniwaSequence/HakoniwaSequence.h>
#include <game/StageScene/StageScene.h>
#include <game/System/Application.h>
#include <game/System/GameSystem.h>
#include <hook/trampoline.hpp>
#include <logger/SocketInterface.h>
#include <utils/Helpers.h>
#include <utils/SpeedbootTwo.hpp>
#include <utils/UsefulPatches.hpp>

#include "rs/util.hpp"

#include "agl/utl.h"

#include <mob/MonsterState.hpp>
#include <spook/SpookState.hpp>

static const char* DBG_FONT_PATH = "DebugData/Font/nvn_font_jis1.ntx";
static const char* DBG_SHADER_PATH = "DebugData/Font/nvn_font_shader_jis1.bin";
static const char* DBG_TBL_PATH = "DebugData/Font/nvn_font_jis1_tbl.bin";

#define IMGUI_ENABLED false

sead::TextWriter* gTextWriter;

void drawDebugWindow() {
  HakoniwaSequence* gameSeq = (HakoniwaSequence*)GameSystemFunction::getGameSystem()->mCurSequence;

  static bool debugOpen = false;
  return;
  ImGui::Begin("Game Debug Window", &debugOpen);
  ImGui::SetWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

  ImGui::Text("Current Sequence Name: %s", gameSeq->getName().cstr());

  static bool showWindow = false;

  if (ImGui::Button("Toggle Demo Window")) {
    showWindow = !showWindow;
  }

  if (showWindow) {
    ImGui::ShowDemoWindow();
  }

  auto curScene = gameSeq->mStageScene;

  bool isInGame = curScene && curScene->mIsAlive;

  if (ImGui::CollapsingHeader("World List")) {
    for (auto& entry : gameSeq->mGameDataHolder.mData->mWorldList->mWorldList) {
      if (ImGui::TreeNode(entry.mMainStageName)) {

        if (isInGame) {
          if (ImGui::Button("Warp to World")) {
            PlayerHelper::warpPlayer(entry.mMainStageName, gameSeq->mGameDataHolder);
          }
        }

        ImGui::BulletText("Clear Main Scenario: %d", entry.mClearMainScenario);
        ImGui::BulletText("Ending Scenario: %d", entry.mEndingScenario);
        ImGui::BulletText("Moon Rock Scenario: %d", entry.mMoonRockScenario);

        if (ImGui::TreeNode("Main Quest Infos")) {
          for (int i = 0; i < entry.mQuestInfoCount; ++i) {
            ImGui::BulletText("Quest %d Scenario: %d", i, entry.mMainQuestIndexes[i]);
          }
          ImGui::TreePop();
        }

        if (ImGui::CollapsingHeader("Database Entries")) {
          for (auto& dbEntry : entry.mStageNames) {
            if (ImGui::TreeNode(dbEntry.mStageName.cstr())) {
              ImGui::BulletText("Stage Category: %s", dbEntry.mStageCategory.cstr());
              ImGui::BulletText("Stage Use Scenario: %d", dbEntry.mUseScenario);

              if (isInGame) {
                ImGui::Bullet();
                if (ImGui::SmallButton("Warp to Stage")) {
                  PlayerHelper::warpPlayer(dbEntry.mStageName.cstr(), gameSeq->mGameDataHolder);
                }
              }

              ImGui::TreePop();
            }
          }
        }

        ImGui::TreePop();
      }
    }
  }

  if (isInGame) {
    StageScene* stageScene = gameSeq->mStageScene;
    PlayerActorBase* playerBase = rs::getPlayerActor(stageScene);

    if (ImGui::Button("Kill Mario")) {
      PlayerHelper::killPlayer(playerBase);
    }
  }

  ImGui::End();
}

struct ReplaceSeadPrint : public ::exl::hook::impl::ReplaceHook<ReplaceSeadPrint> {
  static void Callback(const char* format, ...) {
    va_list args;
    va_start(args, format);
    Logger::log(format, args);
    va_end(args);
  }
};

struct GameSystemInit : public ::exl::hook::impl::TrampolineHook<GameSystemInit> {
  static void Callback(GameSystem* thisPtr) {
    cs::CaptureState::init();
    // alloc<sp::StaticManager>();
    // sp::StaticManager::init();

    sead::Heap* curHeap = sead::HeapMgr::instance()->getCurrentHeap();

    sead::DebugFontMgrJis1Nvn::createInstance(curHeap);

    if (al::isExistFile(DBG_SHADER_PATH) && al::isExistFile(DBG_FONT_PATH) && al::isExistFile(DBG_TBL_PATH)) {
      sead::DebugFontMgrJis1Nvn::instance()->initialize(curHeap, DBG_SHADER_PATH, DBG_FONT_PATH, DBG_TBL_PATH,
                                                        0x100000);
    }

    sead::TextWriter::setDefaultFont(sead::DebugFontMgrJis1Nvn::instance());

    al::DrawSystemInfo* drawInfo = Application::instance()->mDrawInfo;

    agl::DrawContext* context = drawInfo->drawContext;
    agl::RenderBuffer* renderBuffer = drawInfo->getRenderBuffer();

    auto* viewport = alloc<sead::Viewport>(*renderBuffer);

    gTextWriter = alloc<sead::TextWriter>(context, viewport);

    sead::TextWriter::setupGraphics(context);

    gTextWriter->mColor = sead::Color4f(1.f, 1.f, 1.f, 0.8f);

    Orig(thisPtr);

    sead::ScopedCurrentHeapSetter setter(al::getSequenceHeap());
    auto instance = cs::CaptureState::createInstance(al::getSequenceHeap());
        instance->loadState();
    // mob::MonsterState::createInstance(nullptr);
    // mob::MonsterState::init();
  }
};

struct DrawDebugMenu : public ::exl::hook::impl::TrampolineHook<DrawDebugMenu> {
  static void Callback(HakoniwaSequence* thisPtr) {

    Orig(thisPtr);

    if (par::get("DebugDisplay", false)) {

      gTextWriter->beginDraw();

      gTextWriter->setCursorFromTopLeft(sead::Vector2f(10.f, 200.f));
      gTextWriter->printf("FPS: %d\n", static_cast<int>(round(Application::instance()->mFramework->calcFps())));

      // cs::CaptureState::instance()->draw(gTextWriter);

      gTextWriter->endDraw();
    }
  }
};

extern "C" void exl_main(void* x0, void* x1) {
  // while (true) {
  // }
  /* Setup hooking enviroment. */
  exl::hook::Initialize();

  handler::installExceptionHandler([](handler::ExceptionInfo& info) {
    Logger::log("That shit crashed so hard\n");
    //        handler::printCrashReport(info);
    return false;
  });

  // EXL_ASSERT(SocketInterface::instance().init(LOGGER_IP, 3086), "SOCKET SERVER MUST BE RAMING!");
  EXL_ASSERT(SocketInterface::instance().init(LOGGER_IP, 3085), "SOCKET SERVER MUST BE FAMING!");
  //    SocketInterface::instance().waitForConnection();

  EXL_ASSERT(R_SUCCEEDED(nn::fs::MountSdCardForDebug("sd")), "SD card failed to mount...");

  struct WarnOnMalloc : Trampoline<WarnOnMalloc> {
    static void* Callback(size_t size) { return Orig(size); }
  };

  //    for (auto module = nn::ro::detail::g_pAutoLoadList; module != nullptr; module = module->next) {
  //        auto dyn = unsafeRef<Elf64_Dyn*>(module, 0x28);
  //        const char* stringTable = nullptr;
  //        Elf64_Xword soNameOffset = U64_MAX;
  //        for (; dyn->d_tag != DT_NULL; dyn++) {
  //            if (dyn->d_tag == DT_STRTAB) stringTable = reinterpret_cast<const char*>(dyn->d_un.d_ptr);
  //            if (dyn->d_tag == DT_SONAME) soNameOffset = dyn->d_un.d_val;
  //        }
  //
  //        if (stringTable != nullptr && soNameOffset != U64_MAX) {
  //            Logger::log("Module name: %s\n", stringTable + soNameOffset);
  //        }
  //    }
  //    WarnOnMalloc::InstallAtPtr(nn::ro::Module*);
  //    svcBreak(0,0,0);

  runCodePatches();

  GameSystemInit::InstallAtOffset(0x535850);

  // Sead Debugging Overriding

  ReplaceSeadPrint::InstallAtOffset(0xB59E28);

  // Debug Text Writer Drawing

  DrawDebugMenu::InstallAtOffset(0x50F1D8);

  // svcBreak(1, 1, 0);
  nvnImGui::InstallHooks();

  // svcBreak(1, 1, 1);
  // sp::spookyInit();
  // svcBreak(1, 1, 2);
  sb::speedbootPatches();
  // svcBreak(1, 1, 3);
  up::usefulPatchesInit();

  // svcBreak(1, 0, 4);
  nvnImGui::addDrawFunc(drawDebugWindow);
}

extern "C" NORETURN void exl_exception_entry() {
  /* TODO: exception handling */
  EXL_ABORT(0x420);
}
