#include <bonk/BonkProcedure.hpp>
#include <bonk/ModSaveData.hpp>
#include <bonk/StageState.hpp>
#include <heap/seadHeapMgr.h>

namespace bm {
    s32 bonkStepCount = 0;
    Step bonkSteps[30] = {};
    Step& getNextStep() { return bonkSteps[bonkStepCount++]; }
    void executeStep(Step& step) {
        sead::ScopedCurrentHeapSetter setter(al::getSceneHeap());
        switch (step.type) {
            case Step::Type::None:
                break;
            case Step::Type::Enable: {
                auto mod = StageState::findMod(step.getTypeInfo());
                if (!mod->isActive())
                    mod->activate();
                Logger::log("Enabled mod %s\n", step.typeInfo->name());
                break;
            }
            case Step::Type::Modify: {
                auto& func = *step.modifyFunction;
                func(StageState::findMod(step.getTypeInfo()));
                Logger::log("Modified mod %s\n", step.typeInfo->name());
                break;
            }
        }
    }
    bool bonked() {
        auto& save = ModSaveData::instance();
        if (bonkSteps[save.modStep].type == Step::Type::None) {
            Logger::log("Reached maximum steps, no more to execute!\n");
            return false;
        }
        Logger::log("Executing bonk step %d\n", save.modStep);
        executeStep(bonkSteps[save.modStep++]);
        save.save();
        return true;
    }
    void procedureStartup() {
        if (par::get("SaveDisableStartup", false)) return;
        auto save = ModSaveData::instance();
        for (int i = 0; i < save.modStep; ++i)
            executeStep(bonkSteps[i]);
        Logger::log("Loaded with %d bonk steps\n", save.modStep);
    }
} // namespace bm
