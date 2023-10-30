#include <al/Library/Yaml/Writer/ByamlWriter.h>
#include <bonk/mods/RemoveHat.hpp>
#include <game/GameData/GameDataFunction.h>
#include <game/Player/CapFunction.h>
#include <logger/Logger.hpp>

namespace bm {

    namespace {
        MAKE_NERVE_BOTH(RemoveHat, Hide);
        MAKE_NERVE_BOTH(RemoveHat, Show);
    } // namespace
    RemoveHat::RemoveHat() { initNerve(&RemoveHatNrvShow::sInstance, 0); }
    void RemoveHat::sceneStart(const al::ActorInitInfo& initInfo) {
        Mod::sceneStart(initInfo);
        levelStartedWithHat = GameDataFunction::isEnableCap(this);
        Logger::log("Started cap: %s\n", BTOC(levelStartedWithHat));
        al::setNerve(this, &RemoveHatNrvShow::sInstance);
    }
    static RemoveHat* hat;
    void useOldBoolean(al::ByamlWriter* writer, const char* name, bool) {
        writer->addBool(name, hat->levelStartedWithHat);
    }
    struct HatEnabled : public Trampoline<HatEnabled> {
        static void Callback(GameDataHolderWriter writer) {
            Orig(writer);
            hat->levelStartedWithHat = true;
        }
    };
    void RemoveHat::activate() {
        Mod::activate();
        hat = this;
        patch::CodePatcher patcher(0x527634);
        patcher.BranchLinkInst((void*)&useOldBoolean);
        HatEnabled::InstallAtSymbol("_ZN16GameDataFunction9enableCapE20GameDataHolderWriter");
    }
    void RemoveHat::update() {
        if (par::clicked("HatPauseZero")) {
            pauseTimer = 0;
            Logger::log("Current nerve: %s\n", typeid(*mNerveKeeper->mNextNerve).name());
        }
        if (par::clicked("HatEnable")) {
            exeShow();
            GameDataFunction::restartStage(GameDataHolderAccessor(this));
        }
        Mod::update();
    }
    void RemoveHat::control() {
        if (levelStartedWithHat)
            NerveMod::control();
    }
    void RemoveHat::exeHide() {
        getGameDataHolder(this)->mDataFile->mIsEnableCap = false;
        CapFunction::putOnCapPlayer(getMario()->mHackCap, getMario()->mPlayerAnimator);
        pauseForSeconds(par::get<u32>("HatHideSecs", 5));
        Logger::log("Hid cap\n");
        al::setNerve(this, &RemoveHatNrvShow::sInstance);
    }
    void RemoveHat::exeShow() {
        getGameDataHolder(this)->mDataFile->mIsEnableCap = true;
        Logger::log("Showed cap\n");
        getMario()->mHackCap->hide(false);
        getMario()->mPlayerAnimator->forceCapOn();
        pauseForSeconds(par::get<u32>("HatShowSecsBase", 10) + al::getRandom(par::get("HatShowSecsExtra", 20)));
        al::setNerve(this, &RemoveHatNrvHide::sInstance);
    }
    void RemoveHat::sceneEnd() {
        Mod::sceneEnd();
        if (levelStartedWithHat) {
            getGameDataHolder(this)->mDataFile->mIsEnableCap = true;
        }
    }
} // namespace bm