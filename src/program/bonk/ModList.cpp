#include <al/Library/Memory/HeapUtil.h>
#include <bonk/ModSaveData.hpp>
#include <bonk/StageState.hpp>
#include <bonk/mods/Birdyssey.hpp>
#include <bonk/mods/BouncyMario.hpp>
#include <bonk/mods/CameraLock.hpp>
#include <bonk/mods/DetroitBecomeCake.hpp>
#include <bonk/mods/DiscordPing.hpp>
#include <bonk/mods/GravityShift.hpp>
#include <bonk/mods/GreenDemonMod.hpp>
#include <bonk/mods/InputRandomizer.hpp>
#include <bonk/mods/InvisibleMan.hpp>
#include <bonk/mods/JumpyMario.hpp>
#include <bonk/mods/NoOxygen.hpp>
#include <bonk/mods/PlayerConstMod.hpp>
#include <bonk/mods/PoseRandomizer.hpp>
#include <bonk/mods/Preprocessor.hpp>
#include <bonk/mods/RemoveHat.hpp>
#include <bonk/mods/StickDelay.hpp>
#include <bonk/mods/StickInverter.hpp>
#include <bonk/mods/Subscribe.hpp>
#include <bonk/mods/TallMario.hpp>
#include <bonk/mods/TwitchChat.hpp>
#include <bonk/mods/WideMario.hpp>
#include <bonk/mods/WorseGravity.hpp>
#include <heap/seadHeapMgr.h>

namespace bm {
    static bool initialized = false;
    void addMods() {
        if (initialized)
            return;
        initialized = true;

        sead::ScopedCurrentHeapSetter heapSetter(al::getSequenceHeap());
//        bm::addMod<PoseRandomizer>() /*->activate()*/;
//        bm::addMod<PlayerConstMod>() /*->activate()*/;
//        bm::addMod<StickInverter>() /*->activate()*/;
//        bm::addMod<NoOxygen>() /*->activate()*/;
//        bm::addMod<GreenDemonMod>() /*->activate()*/;
//        bm::addMod<DetroitBecomeCake>() /*->activate()*/;
//        bm::addMod<InvisibleMan>() /*->activate()*/;
//        bm::addMod<InputRandomizer>() /*->activate()*/;
//        bm::addMod<StickDelay>() /*->activate()*/;
//        bm::addMod<RemoveHat>() /*->activate()*/;
//        bm::addMod<CameraLock>() /*->activate()*/;
//        bm::addMod<GravityShift>() /*->activate()*/;
//        bm::addMod<WideMario>() /*->activate()*/;
//        bm::addMod<TallMario>() /*->activate()*/;
//        bm::addMod<WorseGravity>() /*->activate()*/;
//        bm::addMod<BouncyMario>() /*->activate()*/;
//        bm::addMod<JumpyMario>()/*->activate()*/;
        bm::addMod<Preprocessor>()->activate();
        bm::addMod<Birdyssey>()->activate();

        ModSaveData::instance().load();
        ModSaveData::instance().save();
    }
} // namespace bm