#pragma once

#include <types.h>

namespace bm {
    struct ModSaveData {
        static const int fileMagic = 0x4B4E4F42;
        static const int currentVersion = 1;
        u32 magic = fileMagic;
        u32 version = currentVersion;
        u32 enabledMods = 0;
        u32 modStep = 0;

        static ModSaveData& instance();
        static void setupSave();

        void load();
        void save();
    };
}
