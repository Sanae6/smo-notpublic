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
    StageState::~StageState() {
        if (hadMario)
            for (auto mod : mods)
                mod->sceneEnd();
        hadMario = false;
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
            if (al::isFirstStep(scene)) stageState(scene).exePlayFirstStep();
            stageState(scene).update(false);
        }
    };

    struct StageSceneControl : public Trampoline<StageSceneControl> {
        static void Callback(StageScene* scene) {
            Orig(scene);
            stageState(scene).update(true);
        }
    };

    struct StageSceneDrawMain : public Trampoline<StageSceneDrawMain> {
        static void Callback(StageScene* scene) {
            Orig(scene);
            //            Logger::log("main draw %s\n", typeid(*scene->getNerveKeeper()->getCurrentNerve()).name());
            //            Logger::log("gain draw %s\n", typeid(StageSceneNrvPlay).name());
            if (isNerve<StageSceneNrvPlay>(scene))
                stageState(scene).draw(scene, al::getSceneDrawContext(scene));
        }
    };

    struct HakoniwaSequenceDrawMain : public Trampoline<HakoniwaSequenceDrawMain> {
        static void Callback(HakoniwaSequence* sequence) {
            Orig(sequence);
//            Logger::log("Draw main\n");
//            if (sequence->mStageScene != nullptr && isSameType<StageState>(sequence->mStageScene) &&
//                isNerve<StageSceneNrvPlay>(sequence->mStageScene)) {
//                StageScene* scene = static_cast<StageScene*>(sequence->mStageScene);
//                Logger::log("draw mnae\n");
//                stageState(scene).draw(scene, al::getSceneDrawContext(scene));
//            }
        }
    };

    void stageStatePatches() {
        StageStateCreate::InstallAtSymbol("_ZN15SceneObjFactory20createSceneObjHolderEv");
        StageStateInit::InstallAtSymbol("_ZN8RootTask4calcEv");
        StageScenePlay::InstallAtSymbol("_ZN10StageScene7exePlayEv");
        StageSceneControl::InstallAtSymbol("_ZN10StageScene7controlEv");
        //        StageSceneDrawMain::InstallAtSymbol("_ZNK10StageScene8drawMainEv");
//        HakoniwaSequenceDrawMain::InstallAtSymbol("_ZNK16HakoniwaSequence8drawMainEv");
    }

} // namespace bm