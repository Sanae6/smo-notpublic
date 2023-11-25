#include <al/Library/Controller/JoyPadUtil.h>
#include <bonk/mods/InputRandomizer.hpp>
#include <controller/seadControllerMgr.h>

namespace bm {
    static sead::PadIdx randoBtns[] = {
        sead::PadIdx::A, sead::PadIdx::X, sead::PadIdx::ZL, sead::PadIdx::L
    };

    struct InputRandomizerHook : public Trampoline<InputRandomizerHook> {
        static bool fuckUpThisFrame;

        static void Callback(sead::ControllerBase* c) {
            Orig(c);
            if (c == sead::ControllerMgr::instance()->getController(al::getMainControllerPort()) && fuckUpThisFrame) {
                fuckUpThisFrame = false;
                for (int i = 0; i < par::get("InputRandoCount", 2); i++)
                    c->mPadHold.setBit((s32)randoBtns[al::getRandom(0, (s32)std::size(randoBtns))]);
                // TODO: Why does mario float sometimes
            }
        }
    };
    bool InputRandomizerHook::fuckUpThisFrame = false;
    void InputRandomizer::activate() {
        Mod::activate();
        InputRandomizerHook::InstallAtSymbol("_ZN2al14NpadController9calcImpl_Ev");
    }
    void InputRandomizer::control() {
        Mod::control();
        InputRandomizerHook::fuckUpThisFrame = true;
        pauseForFrames(par::get("InputRandoCooldown", 90));
    }
} // namespace bm
