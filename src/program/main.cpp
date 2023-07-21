#include "lib.hpp"
#include "imgui_backend/imgui_impl_nvn.hpp"
#include "patches.hpp"
#include "logger/Logger.hpp"
#include "fs.h"
#include "helpers/InputHelper.h"
#include "helpers/PlayerHelper.h"
#include "imgui_nvn.h"
#include "ExceptionHandler.h"

#include <basis/seadRawPrint.h>
#include <devenv/seadDebugFontMgrNvn.h>
#include <filedevice/nin/seadNinSDFileDeviceNin.h>
#include <filedevice/seadFileDeviceMgr.h>
#include <filedevice/seadPath.h>
#include <resource/seadArchiveRes.h>
#include <heap/seadHeapMgr.h>
#include <devenv/seadDebugFontMgrNvn.h>
#include <gfx/seadTextWriter.h>
#include <gfx/seadViewport.h>
#include <heap/seadExpHeap.h>
#include <heap/seadHeapMgr.h>
#include <cmath>
#include <koopa/Koopa.hpp>
#include <prim/seadSafeString.h>
#include <resource/seadArchiveRes.h>
#include <resource/seadResourceMgr.h>

#include <al/Library/File/FileLoader.h>
#include <al/Library/File/FileUtil.h>

#include <al/LiveActor/LiveActorFlag.hpp>
#include <game/GameData/GameDataFunction.h>
#include <game/HakoniwaSequence/HakoniwaSequence.h>
#include <game/StageScene/StageScene.h>
#include <game/System/Application.h>
#include <game/System/GameSystem.h>
#include <gfx/seadPrimitiveRenderer.h>

#include "rs/util.hpp"

#include "agl/utl.h"
#include "al/Library/Controller/JoyPadUtil.h"
#include <al/Library/Nerve/NerveKeeper.h>
#include <al/Library/Player/PlayerHolder.h>

static const char *DBG_FONT_PATH = "DebugData/Font/nvn_font_jis1.ntx";
static const char *DBG_SHADER_PATH = "DebugData/Font/nvn_font_shader_jis1.bin";
static const char *DBG_TBL_PATH = "DebugData/Font/nvn_font_jis1_tbl.bin";

sead::TextWriter *gTextWriter;

void drawDebugWindow() {
    HakoniwaSequence *gameSeq = (HakoniwaSequence *) GameSystemFunction::getGameSystem()->mCurSequence;

    ImGui::Begin("Game Debug Window");
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

    bool isInGame =
            curScene && curScene->mIsAlive;

    if (ImGui::CollapsingHeader("World List")) {
        for (auto &entry: gameSeq->mGameDataHolder.mData->mWorldList->mWorldList) {
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
                    for (auto &dbEntry: entry.mStageNames) {
                        if (ImGui::TreeNode(dbEntry.mStageName.cstr())) {
                            ImGui::BulletText("Stage Category: %s", dbEntry.mStageCategory.cstr());
                            ImGui::BulletText("Stage Use Scenario: %d", dbEntry.mUseScenario);

                            if (isInGame) {
                                ImGui::Bullet();
                                if (ImGui::SmallButton("Warp to Stage")) {
                                    PlayerHelper::warpPlayer(dbEntry.mStageName.cstr(),
                                                             gameSeq->mGameDataHolder);
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
        StageScene *stageScene = gameSeq->mStageScene;
        PlayerActorBase *playerBase = rs::getPlayerActor(stageScene);

        if (ImGui::Button("Kill Mario")) {
            PlayerHelper::killPlayer(playerBase);
        }
    }

    ImGui::End();
}

HOOK_DEFINE_TRAMPOLINE(ControlHook) {
    static void Callback(StageScene *scene) {
        Orig(scene);
    }
};

HOOK_DEFINE_REPLACE(ReplaceSeadPrint) {
    static void Callback(const char *format, ...) {
        va_list args;
        va_start(args, format);
        Logger::log(format, args);
        va_end(args);
    }
};

HOOK_DEFINE_TRAMPOLINE(CreateFileDeviceMgr) {
    static void Callback(sead::FileDeviceMgr *thisPtr) {

        Orig(thisPtr);

        thisPtr->mMountedSd = nn::fs::MountSdCardForDebug("sd").isSuccess();

        sead::NinSDFileDevice *sdFileDevice = new sead::NinSDFileDevice();

        thisPtr->mount(sdFileDevice);
    }
};

HOOK_DEFINE_TRAMPOLINE(RedirectFileDevice) {
    static sead::FileDevice *
    Callback(sead::FileDeviceMgr *thisPtr, sead::SafeString &path, sead::BufferedSafeString *pathNoDrive) {

        sead::FixedSafeString<32> driveName;
        sead::FileDevice *device;

        // Logger::log("Path: %s\n", path.cstr());

        if (!sead::Path::getDriveName(&driveName, path)) {

            device = thisPtr->findDevice("sd");

            if (!(device && device->isExistFile(path))) {

                device = thisPtr->getDefaultFileDevice();

                if (!device) {
                    Logger::log("drive name not found and default file device is null\n");
                    return nullptr;
                }

            } else {
                Logger::log("Found File on SD! Path: %s\n", path.cstr());
            }

        } else
            device = thisPtr->findDevice(driveName);

        if (!device)
            return nullptr;

        if (pathNoDrive != nullptr)
            sead::Path::getPathExceptDrive(pathNoDrive, path);

        return device;
    }
};

HOOK_DEFINE_TRAMPOLINE(FileLoaderLoadArc) {
    static sead::ArchiveRes *
    Callback(al::FileLoader *thisPtr, sead::SafeString &path, const char *ext, sead::FileDevice *device) {

        // Logger::log("Path: %s\n", path.cstr());

        sead::FileDevice *sdFileDevice = sead::FileDeviceMgr::instance()->findDevice("sd");

        if (sdFileDevice && sdFileDevice->isExistFile(path)) {

            Logger::log("Found File on SD! Path: %s\n", path.cstr());

            device = sdFileDevice;
        }

        return Orig(thisPtr, path, ext, device);
    }
};

sead::FileDevice *tryFindNewDevice(sead::SafeString &path, sead::FileDevice *orig) {
    sead::FileDevice *sdFileDevice = sead::FileDeviceMgr::instance()->findDevice("sd");

    if (sdFileDevice && sdFileDevice->isExistFile(path))
        return sdFileDevice;

    return orig;
}

HOOK_DEFINE_TRAMPOLINE(FileLoaderIsExistFile) {
    static bool Callback(al::FileLoader *thisPtr, sead::SafeString &path, sead::FileDevice *device) {
        return Orig(thisPtr, path, tryFindNewDevice(path, device));
    }
};

HOOK_DEFINE_TRAMPOLINE(FileLoaderIsExistArchive) {
    static bool Callback(al::FileLoader *thisPtr, sead::SafeString &path, sead::FileDevice *device) {
        return Orig(thisPtr, path, tryFindNewDevice(path, device));
    }
};

HOOK_DEFINE_TRAMPOLINE(GameSystemInit) {
    static void Callback(GameSystem *thisPtr) {

        sead::Heap *curHeap = sead::HeapMgr::instance()->getCurrentHeap();

        sead::DebugFontMgrJis1Nvn::createInstance(curHeap);

        if (al::isExistFile(DBG_SHADER_PATH) && al::isExistFile(DBG_FONT_PATH) && al::isExistFile(DBG_TBL_PATH)) {
            sead::DebugFontMgrJis1Nvn::instance()->initialize(curHeap, DBG_SHADER_PATH, DBG_FONT_PATH, DBG_TBL_PATH,
                                                              0x100000);
        }

        sead::TextWriter::setDefaultFont(sead::DebugFontMgrJis1Nvn::instance());

        al::GameDrawInfo *drawInfo = Application::instance()->mDrawInfo;

        agl::DrawContext *context = drawInfo->mDrawContext;
        agl::RenderBuffer *renderBuffer = drawInfo->mFirstRenderBuffer;

        sead::Viewport *viewport = new sead::Viewport(*renderBuffer);

        gTextWriter = new sead::TextWriter(context, viewport);

        gTextWriter->setupGraphics(context);

        gTextWriter->mColor = sead::Color4f(1.f, 1.f, 1.f, 0.8f);

        sead::PrimitiveRenderer* renderer = sead::PrimitiveRenderer::instance();

        renderer->setDrawContext(context);
        renderer->setModelMatrix(sead::Matrix34f::ident);

        Orig(thisPtr);

    }
};


extern Koopa* koopa;
extern bool haveIBeenPwned;
extern bool demoage;

const al::Nerve* getPlayerHack();
const char* getPlayerHackPtr();
bool isNerve(al::IUseNerve* nerveUser, const al::Nerve* nerve);
void koopaDrawDebug(sead::PrimitiveRenderer& renderer);
HOOK_DEFINE_TRAMPOLINE(DrawDebugMenu) {
    static void Callback(HakoniwaSequence *thisPtr) {
        Orig(thisPtr);

//        if (thisPtr->mCurrentScene && strstr(typeid(*al::getCurrentNerve(thisPtr)).name(), "Destroy") == nullptr) {
//            al::Scene* scene = thisPtr->mCurrentScene;
//        }

        gTextWriter->beginDraw();
//
        gTextWriter->setCursorFromTopLeft(sead::Vector2f(10.f, 10.f));
        gTextWriter->printf("FPS: %d\n", static_cast<int>(std::round(Application::instance()->mFramework->calcFps())));
        if (koopa){
            auto* flags = (bool*)koopa->getFlags();
            for (int i = 0; i < 12; i++) {
                gTextWriter->printf("Koopa flag %d: %s\n", i, BTOC(flags[i]));
            }
            if (koopa->getNerveKeeper())
                gTextWriter->printf("Nerve name %s\n", typeid(*koopa->getNerveKeeper()->getCurrentNerve()).name());
            gTextWriter->printf("Hack exists %s\n", BTOC(koopa->mPlayerHack));
            gTextWriter->printf("pwned %s\n", BTOC(haveIBeenPwned));
            gTextWriter->printf("demoage %d\n", demoage);
            if (koopa->getSceneInfo() && koopa->getSceneInfo()->mDemoDirector)
                gTextWriter->printf("demo active %s\n", BTOC(rs::isActiveDemo(koopa)));

#define OFFSET(ptr, offset) (((u8*)ptr) + ((size_t)offset))

            if (koopa->getSceneInfo()) {
                auto* player = (PlayerActorHakoniwa*)al::getPlayerActor(koopa, 0);
                if (player && player->getNerveKeeper()) {
                    gTextWriter->printf("Player name %s\n",
                                        typeid(*player->getNerveKeeper()->getCurrentNerve()).name());
                    gTextWriter->printf("Expected name %s\n", getPlayerHackPtr());
                    gTextWriter->printf("Freddy fast bear %s\n", BTOC(isNerve(player, getPlayerHack())));
                    if (isNerve(player, getPlayerHack()) && player->getNerveKeeper()->mStateCtrl->mCurrentState) {
                        gTextWriter->printf("Player name %s\n",
                                        typeid(*player->getNerveKeeper()->mStateCtrl->mCurrentState->state->mNerveKeeper->getCurrentNerve()).name());
                    }
                    gTextWriter->printf("2D %s\n", BTOC(rs::isPlayer2D(player)));
                }
                if (player && player->mHackCap) {
                    gTextWriter->printf("Target info %s\n", BTOC(*(CapTargetInfo**)OFFSET(player->mHackCap, 0x228)));

                    if (*(u8**)OFFSET(player->mHackCap, 0x228) && al::isPadTriggerRight(-1))
                        *(CapTargetInfo**)OFFSET(player->mHackCap, 0x228) = nullptr;
                    if (player->mHackCap->getNerveKeeper())
                        gTextWriter->printf("Player name %s\n",
                                            typeid(*player->mHackCap->getNerveKeeper()->getCurrentNerve()).name());
                }
            }
        }

        gTextWriter->endDraw();

    }
};

void koopaPatchesInit();

extern "C" void exl_main(void *x0, void *x1) {
    /* Setup hooking enviroment. */
    exl::hook::Initialize();

    handler::installExceptionHandler([](handler::ExceptionInfo& info) {
        handler::printCrashReport(info);
        return false;
    });

    Logger::instance().init(LOGGER_IP, 3085);

    runCodePatches();

    GameSystemInit::InstallAtOffset(0x535850);

    // SD File Redirection

    RedirectFileDevice::InstallAtOffset(0x76CFE0);
    FileLoaderLoadArc::InstallAtOffset(0xA5EF64);
    CreateFileDeviceMgr::InstallAtOffset(0x76C8D4);
    FileLoaderIsExistFile::InstallAtSymbol(
            "_ZNK2al10FileLoader11isExistFileERKN4sead14SafeStringBaseIcEEPNS1_10FileDeviceE");
    FileLoaderIsExistArchive::InstallAtSymbol(
            "_ZNK2al10FileLoader14isExistArchiveERKN4sead14SafeStringBaseIcEEPNS1_10FileDeviceE");

    // Sead Debugging Overriding
    ReplaceSeadPrint::InstallAtOffset(0xB59E28);

    // Debug Text Writer Drawing

    DrawDebugMenu::InstallAtOffset(0x50F1D8);

    // General Hooks

    ControlHook::InstallAtSymbol("_ZN10StageScene7controlEv");

    // ImGui Hooks
    nvnImGui::InstallHooks();

    koopaPatchesInit();
}

extern "C" NORETURN void exl_exception_entry() {
    /* TODO: exception handling */
    EXL_ABORT(0x420);
}
