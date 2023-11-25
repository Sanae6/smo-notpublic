#include <al/Library/Memory/HeapUtil.h>
#include <bonk/BonkProcedure.hpp>
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
#include <bonk/mods/MoonMode.hpp>
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
        addModStep<WideMario>();                 // scale mario x/z * 3
        addModStep<MoonMode>();                  // moon playerconst (moon kingdom gravity)
        addModStep<GreenDemonMod>();             // beach ball chases mario forever (green demon sm64)
        addModStep<Preprocessor>();              // post-processing (snapshot mode filters)
        addModStep<BouncyMario>();               // mario stretches along velocity
        addModStep<NoOxygen>();                  // oxygen meter always counting down
        addModStep<InvisibleMan>();              // mario becomes invisible (prerequisite for cake)
        addModStep<CameraLock>();                // lock camera in place for x seconds with cooldown of y seconds
        addModStep<RemoveHat>();                 // steal hat for x seconds with cooldown of y seconds
        addModStep<StickDelay>();                // delay all inputs (not just stick) by x frames
        modifyStep<Preprocessor>([](auto& mod) { // enables randomized filters (default just fisheye lens)
            mod.forceSingleFilter = false;
        });
        addModStep<GravityShift>();      // give mario a random gravity vector
        addModStep<DetroitBecomeCake>(); // mario becomes a cake
        addModStep<JumpyMario>();      // always enable jump code so mario jumps like the ground is always a trampoline
        addModStep<StickInverter>();   // invert stick inputs
        addModStep<TallMario>();       // scale mario y * 3
        addModStep<InputRandomizer>(); // randomly press one button every 360 frames
        addModStep<WorseGravity>();    // literally just playerconst modifier on all gravity
        modifyStep<DetroitBecomeCake>([](auto& mod) { // turn mario into a globe
            mod.isGlobe = true;
            if (mod.inScene()) {
                Logger::log("Cake models %p %p\n", mod.cakeModel, mod.globeModel);
                mod.cakeModel->kill();
                mod.globeModel->appear();
            }
        });
        addModStep<Birdyssey>(); // birds fly around and damage mario like cuckoos from zelda

        //        addModStep<PlayerConstMod>(); // randomize playerconst
        //        addModStep<PoseRandomizer>(); // randomize poses (pos, scale, velocity) of all actors

        ModSaveData::instance().load();
        procedureStartup();
        ModSaveData::instance().save();
    }
} // namespace bm