#include <bonk/ModSaveData.hpp>
#include <bonk/StageState.hpp>
#include <bonk/mods/Preprocessor.hpp>
#include <game/GameData/GameDataFunction.h>
#include <logger/Logger.hpp>

namespace bm {
    void Preprocessor::apply() {
        updateAlways = true;
        filter = al::getRandom(1, 18);
        al::validatePostProcessingFilter(stageState(this).stageScene);
    }

    void Preprocessor::control() {
        auto& save = ModSaveData::instance();
        if (save.filtersDisableTimer++ > 14400) {
            return;
        }
        if (save.filtersDisableTimer % 3600 == 0) {
            save.save();
        }

        al::validatePostProcessingFilter(stageState(this).stageScene);
        s32 appliedFilter = filter;
        if (forceSingleFilter || par::get("PreprocOverride", false))
            appliedFilter = par::get("PreprocFilter", 7);
        auto postProc = unsafeRef<al::PostProcessingFilter*>(sceneInfo.mGraphicsSystemInfo, 0x2d0);
        postProc->currentPreset = appliedFilter;

        // support world based view depth params
        auto vdd = postProc->viewDepthDrawer;
        unsafeRef<struct ViewDepthDrawParam*>(vdd, 0x10) = nullptr;
        if (appliedFilter == 9) {
            int world = GameDataFunction::getCurrentWorldId(GameDataHolderAccessor(getGameDataHolder()));
            auto vddParams =
                unsafeRef<ViewDepthDrawParam**>(stageState(this).stageScene->mControllerGuideSnapshotCtrl, 0x48);
            auto vddParamsCount = unsafeRef<int>(stageState(this).stageScene->mControllerGuideSnapshotCtrl, 0x50);
            if (world > -1 && world < vddParamsCount) {
                unsafeRef<ViewDepthDrawParam*>(vdd, 0x10) = vddParams[world];
            }
        }
    }
    void Preprocessor::deactivate() { Mod::deactivate();
        al::invalidatePostProcessingFilter(stageState(this).stageScene);
    }
} // namespace bm
