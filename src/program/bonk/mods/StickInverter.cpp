#include <bonk/mods/StickInverter.hpp>

namespace bm {
    static float modifier = 1.0f;
    void invert(sead::Vector2f& out) { out *= modifier; }
    namespace {
        MAKE_NERVE_BOTH(StickInverter, Enable);
        MAKE_NERVE_BOTH(StickInverter, Disable);
    } // namespace
    StickInverter::StickInverter() : NerveMod() {
        initNerve(&StickInverterNrvEnable::sInstance, 0);
    }
    void StickInverter::activate() {
        Mod::activate();
        patch::CodePatcher patcher(0x861968);
        patcher.BranchInst((void*)&invert);
        patcher.Seek(0x8618d4);
        patcher.BranchInst((void*)&invert);
    }
    void StickInverter::deactivate() { Mod::deactivate();
        patch::CodePatcher patcher(0x861968);
        patcher.Write(inst::Ret());
        patcher.Seek(0x8618d4);
        patcher.Write(inst::Ret());
    }
    void StickInverter::sceneStart(const al::ActorInitInfo& initInfo) {
        Mod::sceneStart(initInfo);
        if (active)
            al::setNerve(this, &StickInverterNrvDisable::sInstance);
        unpause();
    }
    void StickInverter::exeEnable() {
        modifier = -1.0f;
        pauseForSeconds(par::get("InvertOnSecs", 10));
        al::setNerve(this, &StickInverterNrvDisable::sInstance);
    }
    void StickInverter::exeDisable() {
        modifier = 1.0f;
        pauseForSeconds(par::get("InvertOffSecs", 10));
        al::setNerve(this, &StickInverterNrvEnable::sInstance);
    }
} // namespace bm
