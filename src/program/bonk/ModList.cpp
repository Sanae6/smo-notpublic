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
#include <helpers/InputHelper.h>
#include <imgui.h>
#include <imgui_nvn.h>

namespace bm {
    static bool initialized = false;
    void addMods() {
        if (initialized)
            return;
        initialized = true;

        sead::ScopedCurrentHeapSetter heapSetter(al::getSequenceHeap());
        addModStep<WideMario>("Wide Mario");                 // scale mario x/z * 3
        addModStep<MoonMode>("Moon Gravity");                // moon playerconst (moon kingdom gravity)
        addModStep<GreenDemonMod>("A Dangerous Volleyball"); // beach ball chases mario forever (green demon sm64)
        addModStep<Preprocessor>("Retro Mode");   // post-processing (snapshot mode filters)
        addModStep<BouncyMario>("Bouncy Mario");             // mario stretches along velocity
        addModStep<NoOxygen>("Aquatic Mario");               // oxygen meter always counting down
        addModStep<InvisibleMan>("Invisible Man");           // mario becomes invisible (prerequisite for cake)
        addModStep<CameraLock>("Camera Freeze"); // lock camera in place for x seconds with cooldown of y seconds
        addModStep<RemoveHat>("Disable Cappy"); // steal hat for x seconds with cooldown of y seconds
        addModStep<StickDelay>("Delayed Inputs");     // delay all inputs (not just stick) by x frames
        modifyStep<Preprocessor>("All Filters",
                                 [](auto& mod) { // enables randomized filters (default just fisheye lens)
                                     mod.forceSingleFilter = false;
                                     ModSaveData::instance().filtersDisableTimer = 0;
                                 });
        addModStep<GravityShift>("Random Gravity");    // give mario a random gravity vector
        addModStep<DetroitBecomeCake>("Cake Mario");      // mario becomes a cake
        addModStep<JumpyMario>("Trampoline Land"); // always enable jump code so mario jumps like
                                                                         // the ground is always a trampoline
        addModStep<StickInverter>("Inverted Sticks");                     // invert stick inputs
        addModStep<TallMario>("Tall Cake Mario");                           // scale mario y * 3
        addModStep<InputRandomizer>(
            "Random Button Inputs");    // randomly press one button every 360 frames
        addModStep<WorseGravity>("Heavier Gravity"); // literally just playerconst modifier on all gravity
        modifyStep<DetroitBecomeCake>("Globe Mario",
                                      [](auto& mod) { // turn mario into a globe
                                          mod.isGlobe = true;
                                          if (mod.inScene()) {
                                              Logger::log("Cake models %p %p\n", mod.cakeModel, mod.globeModel);
                                              mod.cakeModel->kill();
                                              mod.globeModel->appear();
                                          }
                                      });
        addModStep<Birdyssey>(
            "Angry Birds"); // birds fly around and damage mario like cuckoos from zelda

        //        addModStep<PlayerConstMod>(); // randomize playerconst
        //        addModStep<PoseRandomizer>(); // randomize poses (pos, scale, velocity) of all actors

        ModSaveData::instance().load();
        procedureStartup();
        ModSaveData::instance().save();

        nvnImGui::addDrawFunc([]() {
            if (InputHelper::isInputToggled()) {
                if (ImGui::Begin("Toggle Menu", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {
                    if (ModSaveData::instance().modStep == 0) {
                        ImGui::Text("Nothing yet...");
                    } else
                        for (int i = 0; i < ModSaveData::instance().modStep; i++) {
                            auto& step = bonkSteps[i];
                            if (step.type != Step::Type::Enable || step.text == nullptr)
                                continue;

                            bool v = ModSaveData::instance().isModDisabled(i);
                            bool o = v;
                            if (ImGui::Checkbox(step.text, &v)) {
                                if (o != v) {
                                    auto mod = StageState::findMod(step.getTypeInfo());
                                    if (v)
                                        mod->deactivate();
                                    else
                                        mod->activate();
                                }
                                ModSaveData::instance().setModDisabled(i, v);
                            }
                        }
                }
                ImGui::End();
            }
        });
    }
} // namespace bm