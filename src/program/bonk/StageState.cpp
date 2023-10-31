#include <al/Library/Memory/HeapUtil.h>
#include <al/Library/Player/PlayerHolder.h>
#include <al/Library/Scene/SceneUtil.h>
#include <bonk/ForwardDecls.hpp>
#include <bonk/StageState.hpp>
#include <bonk/mods/PoseRandomizer.hpp>
#include <game/HakoniwaSequence/HakoniwaSequence.h>
#include <game/Player/PlayerActorHakoniwa.h>
#include <game/StageScene/StageScene.h>
#include <game/System/GameSystem.h>
#include <logger/Logger.hpp>
#include <logger/SocketInterface.h>
#include <utils/Helpers.h>

class RootTask;
namespace bm {
    ModList StageState::mods = {};
    bool StageState::isSceneWithMario(const al::ActorInitInfo& initInfo) {
        al::PlayerHolder* playerHolder = initInfo.mActorSceneInfo.mPlayerHolder;
        return playerHolder->getPlayerNum() > 0 && isSameType<PlayerActorHakoniwa>(playerHolder->tryGetPlayer(0));
    }

    void StageState::initAfterPlacementSceneObj(const al::ActorInitInfo& initInfo) {
        if (!isSceneWithMario(initInfo))
            return;
        hadMario = true;
        for (auto mod : mods)
            mod->sceneStart(initInfo);
    }
    void StageState::update(bool control) {
        if (hadMario)
            for (auto mod : mods)
                if (mod->shouldUpdateOnControl() == control)
                    mod->update();
    }
    void StageState::draw(StageScene* scene, agl::DrawContext* drawContext) {
        if (hadMario)
            for (auto mod : mods)
                mod->renderToScreen(scene, drawContext);
    }
    void StageState::exePlayFirstStep() {
        if (hadMario)
            for (auto mod : mods)
                mod->exePlayActivated();
    }
    void StageState::sceneEnd() {
        if (hadMario)
            for (auto mod : mods)
                mod->sceneEnd();
        hadMario = false;
    }

    struct StageStateCreate : public Trampoline<StageStateCreate> {
        static al::SceneObjHolder* Callback() {
            auto* holder = Orig();

            holder->setSceneObj(new (al::getSceneHeap(), 8) StageState(), 0x40);
            return holder;
        }
    };

    void addMods();
    struct StageStateInit : public Trampoline<StageStateInit> {
        static void Callback(RootTask* task) {
            Orig(task);
            addMods();
        }
    };

    struct StageScenePlay : public Trampoline<StageScenePlay> {
        static void Callback(StageScene* scene) {
            Orig(scene);
            if (al::isFirstStep(scene))
                stageState(scene).exePlayFirstStep();
            stageState(scene).update(false);
        }
    };

    struct StageSceneControl : public Trampoline<StageSceneControl> {
        static void Callback(StageScene* scene) {
            Orig(scene);
            if (al::isIntervalStep(scene, 60, 0)) {
                Packet packet(PacketType::Ping, 0);
                SocketInterface::instance().send(&packet);
            }
            stageState(scene).update(true);
        }
    };

    struct StageSceneDestruction : public Trampoline<StageSceneDestruction> {
        static void Callback(HakoniwaSequence* sequence) {
            if (al::isFirstStep(sequence) && isSameType<StageScene>(sequence->getCurrentScene())) {
            }
            Orig(sequence);
        }
    };

    void stageStatePatches() {
        StageStateCreate::InstallAtSymbol("_ZN15SceneObjFactory20createSceneObjHolderEv");
        StageStateInit::InstallAtSymbol("_ZN8RootTask4calcEv");
        StageScenePlay::InstallAtSymbol("_ZN10StageScene7exePlayEv");
        StageSceneControl::InstallAtSymbol("_ZN10StageScene7controlEv");
        StageSceneDestruction::InstallAtSymbol("_ZN16HakoniwaSequence10exeDestroyEv");
        //        StageSceneDrawMain::InstallAtSymbol("_ZNK10StageScene8drawMainEv");
        //        HakoniwaSequenceDrawMain::InstallAtSymbol("_ZNK16HakoniwaSequence8drawMainEv");
    }

} // namespace bm