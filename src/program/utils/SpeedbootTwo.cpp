#include <al/Library/Memory/HeapUtil.h>
#include <al/Library/Nerve/Nerve.h>
#include <al/Library/Nerve/NerveExecutor.h>
#include <al/Library/Nerve/NerveKeeper.h>
#include <al/Library/Nerve/NerveSetupUtil.h>
#include <al/Library/Play/Layout/WipeHolder.h>
#include <game/GameData/GameDataFunction.h>
#include <game/HakoniwaSequence/HakoniwaSequence.h>
#include <lib.hpp>
#include <logger/Logger.hpp>
// #include <utils/AudioWrap.hpp>
#include "Helpers.h"
#include <logger/Params.h>
#include <utils/SpeedbootTwo.hpp>

namespace sb {
    struct SpeedbootNerve : public al::Nerve, public al::NerveExecutor {
        HakoniwaSequence* sequence;
        ChangeStageInfo changeStageInfo;
        SpeedbootNerve(HakoniwaSequence* sequence);

        void execute(al::NerveKeeper* keeper) const override { const_cast<SpeedbootNerve*>(this)->updateNerve(); }
        void executeOnEnd(al::NerveKeeper* keeper) const override { free(const_cast<SpeedbootNerve*>(this)); }

        void exeLoad();
        void exeFade();
    };
    namespace {
        MAKE_NERVE(SpeedbootNerve, Load);
        MAKE_NERVE(SpeedbootNerve, Fade);
        MAKE_NERVE(HakoniwaSequence, LoadStage);
    } // namespace

    SpeedbootNerve::SpeedbootNerve(HakoniwaSequence* sequence)
        : al::NerveExecutor("Speedboot"), sequence(sequence),
        // DonsukeExStage:main_exit
          changeStageInfo(sequence->mGameDataHolder, "main_exit", "DonsukeExStage", false, -1,
                          ChangeStageInfo::UNK) {
        initNerve(&SpeedbootNerveNrvLoad::sInstance, 0);
//        Logger::log("Speedbooting into %s:%s\n", par::get("SpeedyStage", "DonsukeExStage"), par::get("SpeedyChangeId", "DonsukeExStage"));
        Logger::log("Speedbooting into %s:%s\n", changeStageInfo.changeStageName.cstr(), changeStageInfo.changeStageId.cstr());
    }
    void SpeedbootNerve::exeLoad() {
        if (al::isFirstStep(this)) {
            Logger::log("Starting load\n");
            sequence->mInitThread->start();
        }

        if (sequence->mInitThread->isDone()) {
            Logger::log("Done loading, fading out\n");
            al::setNerve(this, &SpeedbootNerveNrvFade::sInstance);
        }
    }
    void SpeedbootNerve::exeFade() {
        if (al::isFirstStep(this)) {
            sequence->mWipeHolder->startClose("FadeWhite", -1);
        }

        if (sequence->mWipeHolder->isCloseEnd()) {
            sead::ScopedCurrentHeapSetter setter(al::getSequenceHeap());
            Logger::log("Faded out, loading\n");
            sequence->mGameDataHolder.mData->changeNextStage(&changeStageInfo, 0);
            al::setNerve(sequence, &HakoniwaSequenceNrvLoadStage::sInstance);
        }
    }

    struct SetupSpeedbootNerve : exl::hook::impl::TrampolineHook<SetupSpeedbootNerve> {
        static void Callback(HakoniwaSequence* sequence, const al::SequenceInitInfo& initInfo) {
            Orig(sequence, initInfo);
            if (!par::get("SpeedyEnabled", false)) return;
            sequence->getNerveKeeper()->mNextNerve = alloc<SpeedbootNerve>(sequence);
            sequence->getNerveKeeper()->mStep = -1;
            Logger::log("Initialized speed booting\n");
        }
    };

    void speedbootPatches() {
        SetupSpeedbootNerve::InstallAtSymbol("_ZN16HakoniwaSequence4initERKN2al16SequenceInitInfoE");
    }
} // namespace sb
