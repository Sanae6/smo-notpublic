#include <bonk/ForwardDecls.hpp>
#include <bonk/ModSaveData.hpp>
#include <fs/fs_files.hpp>
#include <logger/Logger.hpp>
#include <logger/Params.h>
#include <result.h>

namespace bm {
    static const char* path = "sd:/BonkSaveData.bin";
    ModSaveData& ModSaveData::instance() {
        static ModSaveData saveData = {};
        return saveData;
    }
    void ModSaveData::load() {

        nn::fs::FileHandle handle{};
        nn::Result result = nn::fs::OpenFile(&handle, path, nn::fs::OpenMode_Read);
        if (result.isFailure()) {
            Logger::log("Mod save data not found. (0x%x)\n", result.value);
            return;
        }
        s64 fileSize;
        nn::fs::GetFileSize(&fileSize, handle);
        if (fileSize < sizeof(u32) * 2) {
            Logger::log("Mod save data is invalid, size too small for header\n");
            nn::fs::CloseFile(handle);
            return;
        }
        ModSaveData intermediate;
        result = nn::fs::ReadFile(handle, 0, &intermediate, sizeof(ModSaveData));
        nn::fs::CloseFile(handle);
        if (result.isFailure()) {
            Logger::log("Mod save data could not be read. (0x%x)\n", result.value);
            return;
        }

        if (intermediate.magic != fileMagic) {
            Logger::log("Mod save data has invalid magic!\n");
            return;
        }

        if (intermediate.version > currentVersion) {
            Logger::log("Mod save data has greater version than current\n");
            return;
        }

        *this = intermediate;
        if (par::get("SaveOverrideStep", false)) {
            int prev = modStep;
            modStep = par::get("SaveCurrentStep", 0);
            Logger::log("Overwrote mod step %d from %d\n", modStep, prev);
        }

        Logger::log("Loaded save!\n");
    }
    void ModSaveData::save() {
        Logger::log("Creating file %s\n", path);
        nn::fs::CreateFile(path, sizeof(ModSaveData));
        nn::fs::FileHandle handle{};
        Logger::log("Loading file %s\n", path);
        nn::Result result = nn::fs::OpenFile(&handle, path, nn::fs::OpenMode_Write);
        if (result.isFailure()) {
            Logger::log("Failed to open file! (%x)\n", result.value);
            return;
        }

        Logger::log("Writing file %s\n", path);
        result = nn::fs::WriteFile(handle, 0, this, sizeof(ModSaveData), nn::fs::WriteOption::CreateOption(nn::fs::WriteOptionFlag_Flush));
        if (result.isFailure()) {
            Logger::log("Failed to write to file! (%x)\n", result.value);
        }
//        nn::fs::SetFileSize(handle, sizeof(ModSaveData));
        nn::fs::CloseFile(handle);
//        nn::fs::CommitSaveData("save");
        Logger::log("Saved mod save data!\n");
    }
} // namespace bm