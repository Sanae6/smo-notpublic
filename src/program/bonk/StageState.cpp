#include <al/Library/Memory/HeapUtil.h>
#include <al/Library/Nerve/NerveStateBase.h>
#include <al/Library/Player/PlayerHolder.h>
#include <al/Library/Scene/SceneUtil.h>
#include <basis/seadNew.h>
#include <bonk/BonkProcedure.hpp>
#include <bonk/ForwardDecls.hpp>
#include <bonk/StageState.hpp>
#include <bonk/mods/PoseRandomizer.hpp>
#include <game/HakoniwaSequence/HakoniwaSequence.h>
#include <game/Player/PlayerActorHakoniwa.h>
#include <game/StageScene/StageScene.h>
#include <game/System/Application.h>
#include <game/System/GameSystem.h>
#include <helpers/PlayerHelper.h>
#include <logger/Logger.hpp>
#include <logger/SocketInterface.h>
#include <random/seadGlobalRandom.h>
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
        player = static_cast<PlayerActorHakoniwa*>(initInfo.mActorSceneInfo.mPlayerHolder->getPlayer(0));
        sead::GlobalRandom::instance()->init();
        for (auto mod : mods)
            mod->sceneStart(initInfo);
        Logger::log("Initialized mods for scene!\n");
    }
    void StageState::update(bool control) {
        if (hasMario()) {
            if (!control && player->mPlayerTrigger->isOn(PlayerTrigger::EActionTrigger::WallDamage)) {
                Logger::log("Mario bonked!\n");
                if (bonked())
                    for (auto mod : mods)
                        mod->marioBonked();
            }
            if (control && par::clicked("KillMario")) {
                PlayerHelper::killPlayer(player);
            }
            for (auto mod : mods)
                if (mod->shouldUpdateOnControl() == control)
                    mod->update();
        }
    }
    void StageState::draw(StageScene* scene, agl::DrawContext* drawContext) {
        if (hasMario())
            for (auto mod : mods)
                mod->renderToScreen(scene, drawContext);
    }
    void StageState::exePlayFirstStep() {
        if (hasMario())
            for (auto mod : mods)
                mod->exePlayActivated();
    }
    void StageState::sceneEnd(bool cleanResources) {
        if (hasMario())
            for (auto mod : mods)
                mod->sceneEnd(cleanResources);
        player = nullptr;
    }

    struct StageStateCreate : public Trampoline<StageStateCreate> {
        static al::SceneObjHolder* Callback(al::Scene* scene, al::SceneObjHolder* holder) {
            Orig(scene, holder);

            sead::ScopedCurrentHeapSetter setter(al::getSceneHeap());
            auto state = alloc<StageState>();
            if (isSameType<StageScene>(scene))
                state->stageScene = static_cast<StageScene*>(scene);
            holder->setSceneObj(state, 0x40);
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

    struct HakoniwaSequenceDeleteScene : public al::NerveStateBase {
        al::Sequence* sequence;
        al::Scene* scene;
        al::AsyncFunctorThread functorThread;
        bool unloadsResources;
    };

    struct StageSceneDestruction : public Trampoline<StageSceneDestruction> {
        static void Callback(StageScene* scene) {
            stageState(scene).sceneEnd(true);
            Orig(scene);
        }
    };

    void stageStatePatches() {
        StageStateCreate::InstallAtSymbol("_ZN2al5Scene18initSceneObjHolderEPNS_14SceneObjHolderE");
        StageStateInit::InstallAtSymbol("_ZN8RootTask4calcEv");
        StageScenePlay::InstallAtSymbol("_ZN10StageScene7exePlayEv");
        StageSceneControl::InstallAtSymbol("_ZN10StageScene7controlEv");
        StageSceneDestruction::InstallAtSymbol("_ZN10StageSceneD1Ev");
    }

} // namespace bm