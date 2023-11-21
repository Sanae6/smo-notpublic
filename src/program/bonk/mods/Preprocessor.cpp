#include <bonk/StageState.hpp>
#include <bonk/mods/Preprocessor.hpp>
#include <logger/Logger.hpp>

namespace bm {
    static constexpr const int unfavorables[] = {6, 7, 8
                                                 , 17};
    void Preprocessor::apply() {
        do {
            //            filter = al::getRandom(1, 18);
            filter = 7;
            // filter++;
            Logger::log("trying filter %d", filter);
            //        } while (std::find(std::begin(unfavorables), std::end(unfavorables), filter) != std::end(unfavorables));
        } while (false);
        Logger::log("Random filter %d\n", filter);
    }

    void Preprocessor::control() {
        al::validatePostProcessingFilter(stageState(this).stageScene);
        if (par::get("PreprocOverride", false))
            filter = par::get("PreprocFilter", 15);
        unsafeRef<al::PostProcessingFilter*>(sceneInfo.mGraphicsSystemInfo, 0x2d0)->currentPreset = filter;
    }
} // namespace bm
