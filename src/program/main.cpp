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
#include <prim/seadSafeString.h>
#include <heap/seadHeapMgr.h>
#include <devenv/seadDebugFontMgrNvn.h>
#include <gfx/seadTextWriter.h>
#include <gfx/seadViewport.h>

#include <al/Library/File/FileLoader.h>
#include <al/Library/File/FileUtil.h>

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

#include "SwapMemory.hpp"
#include "agl/utl.h"

static const char *DBG_FONT_PATH = "DebugData/Font/nvn_font_jis1.ntx";
static const char *DBG_SHADER_PATH = "DebugData/Font/nvn_font_shader_jis1.bin";
static const char *DBG_TBL_PATH = "DebugData/Font/nvn_font_jis1_tbl.bin";

HOOK_DEFINE_REPLACE(ReplaceSeadPrint) {
    static void Callback(const char *format, ...) {
        va_list args;
        va_start(args, format);
        Logger::log(format, args);
        va_end(args);
    }
};

extern "C" void exl_main(void *x0, void *x1) {
    /* Setup hooking enviroment. */
    exl::hook::Initialize();
//
//    handler::installExceptionHandler([](handler::ExceptionInfo& info) {
//        Logger::log("That shit crashed so hard\n");
////        handler::printCrashReport(info);
//        return false;
//    });

    EXL_ASSERT(R_SUCCEEDED(nn::fs::MountSdCardForDebug("sd")), "SD card failed to mount...");

    runCodePatches();
    ReplaceSeadPrint::InstallAtOffset(0xB59E28);
    sb::speedbootPatches();
    up::usefulPatchesInit();
    swap::initSwap();

    // ImGui Hooks
//#if IMGUI_ENABLED
//    nvnImGui::InstallHooks();
//
//    nvnImGui::addDrawFunc(drawDebugWindow);
//#endif

}

extern "C" NORETURN void exl_exception_entry() {
    /* TODO: exception handling */
    EXL_ABORT(0x420);
}
