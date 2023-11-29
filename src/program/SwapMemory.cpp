#include <ExceptionHandler.h>
#include <heap/seadArena.h>
#include <init.h>
#include <logger/Logger.hpp>
#include <logger/SocketInterface.h>
#include <nn/fs/fs_files.hpp>
#include <nn/fs/fs_mount.hpp>
#include <nn/oe.h>
#include <program/SwapMemory.hpp>
#include <utils/Helpers.h>

namespace swap {
//    constexpr const int swapPageCount = 0x1000;
    constexpr const u64 allocatorSize = 0x2400000;
    constexpr const u64 gameRecordingSize = 0x6000000;
//    constexpr const u64 meelocSize = 0x100000;
    constexpr const u64 totalHeapSize = 0x100000000;
    static sead::Arena pageArena;
    static nn::fs::FileHandle fileHandle;

    struct SwapStart : Replace<SwapStart> {
        static void Callback(sead::Arena* arena) {
            arena->mStart = pageArena.mStart + allocatorSize + gameRecordingSize;
            arena->mSize = pageArena.mSize - (allocatorSize + gameRecordingSize);
        }
    };

    struct Startup : Replace<Startup> {
        static void Callback() {
            Logger::log("Startup\n");
            nn::init::InitializeAllocator(pageArena.mStart, allocatorSize);
            nn::oe::EnableGamePlayRecording(pageArena.mStart + allocatorSize, gameRecordingSize);
        }
    };

    void initSwap() {
//        handler::installExceptionHandler([](handler::ExceptionInfo& info) {
//            if (info.type == handler::ExceptionType::DataAbort && reinterpret_cast<u8*>(info.far) >= pageArena.mStart &&
//                reinterpret_cast<u8*>(info.far) < pageArena.mStart + pageArena.mSize) {
//                nn::Result res = svcMapPhysicalMemory((void*)ALIGN_DOWN(info.far, PAGE_SIZE), PAGE_SIZE);
//                if (R_FAILED(res)) svcBreak(res, 0x12, 0x34);
//                return true;
//            }
//            return false;
//        });
//        SwapStart::InstallAtSymbol("_ZN4sead5Arena10initializeEm");
        SocketInterface::instance().waitForConnection();

        R_ABORT_UNLESS(nn::fs::MountSdCardForDebug("swap"))
        Logger::log("opening swap:/smo/swap.bin\n");
        if (R_FAILED(nn::fs::OpenFile(&fileHandle, "swap:/smo/swap.bin", nn::fs::OpenMode_ReadWrite))) {
            Logger::log("creating swap:/smo/swap.bin\n");
            R_ABORT_UNLESS(nn::fs::CreateFile("swap:/smo/swap.bin", totalHeapSize))
            Logger::log("trying to open swap:/smo/swap.bin again\n");
            R_ABORT_UNLESS(nn::fs::OpenFile(&fileHandle, "swap:/smo/swap.bin", nn::fs::OpenMode_ReadWrite))
        }
        Logger::log("opened swap:/smo/swap.bin\n");

        auto swapAddr = (uintptr_t)virtmemFindAlias(totalHeapSize, 0);
        pageArena.mStart = reinterpret_cast<u8*>(swapAddr);
        pageArena.mSize = totalHeapSize;
    }
} // namespace swap

extern "C" void nninitInitializeAbortObserver() {
    EXL_ASSERT(SocketInterface::instance().init(LOGGER_IP, 3085), "SOCKET SERVER MUST BE GAMING!");
    SocketInterface::instance().waitForConnection();

    swap::Startup::InstallAtSymbol("nninitStartup");
    Logger::log("Abort observer!\n");
}