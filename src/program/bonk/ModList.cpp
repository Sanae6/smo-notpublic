#include <al/Library/Memory/HeapUtil.h>
#include <bonk/StageState.hpp>
#include <bonk/mods/DetroitBecomeCake.hpp>
#include <bonk/mods/GreenDemonMod.hpp>
#include <bonk/mods/NoOxygen.hpp>
#include <bonk/mods/PlayerConstMod.hpp>
#include <bonk/mods/PoseRandomizer.hpp>
#include <bonk/mods/StickInverter.hpp>
#include <bonk/mods/Subscribe.hpp>
#include <heap/seadHeapMgr.h>

namespace bm {
    static bool initialized = false;
    void addMods() {
        if (initialized)
            return;
        initialized = true;

        sead::ScopedCurrentHeapSetter heapSetter(al::getSequenceHeap());
        bm::addMod<PoseRandomizer>()->activate();
        bm::addMod<PlayerConstMod>()/*->activate()*/;
//        bm::addMod<Subscribe>()/*->activate()*/;
        bm::addMod<StickInverter>()/*->activate()*/;
        bm::addMod<NoOxygen>()/*->activate()*/;
        bm::addMod<GreenDemonMod>()/*->activate()*/;
        bm::addMod<DetroitBecomeCake>()->activate();
    }
} // namespace bm