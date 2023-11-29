#pragma once

#include <types.h>
#include <prim/seadBitFlag.h>

namespace bm {
    struct ModSaveData {
        static const int fileMagic = 0x4B4E4F42;
        static const int currentVersion = 1;
        u32 magic = fileMagic;
        u32 version = currentVersion;
        u32 modStep = 0;
        u32 filtersDisableTimer = 0;
        sead::BitFlag32 activeMods = 0;

        static ModSaveData& instance();

        void load();
        void save();

        void setModActive(class Mod* mod, bool active);
    };
}
