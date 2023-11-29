#include <bonk/mods/JumpyMario.hpp>

namespace bm {
    static bool jumpy = false;
    struct JumpCodeAlwaysActive : Trampoline<JumpCodeAlwaysActive> {
        static bool Callback(const IUsePlayerCollision* collision) {
            if (jumpy)
                return true;
            return Orig(collision);
        }
    };
    namespace {
        MAKE_NERVE_BOTH(JumpyMario, Enable);
        MAKE_NERVE_BOTH(JumpyMario, Disable);
    } // namespace
    JumpyMario::JumpyMario() {
        JumpCodeAlwaysActive::InstallAtSymbol("_ZN2rs19isCollisionCodeJumpEPK19IUsePlayerCollision");
    }
    void JumpyMario::activate() {
        Mod::activate();
        initNerve(&JumpyMarioNrvEnable::sInstance, 0);
    }
    void JumpyMario::sceneStart(const al::ActorInitInfo& initInfo) {
        Mod::sceneStart(initInfo);
        if (active)
            al::setNerve(this, &JumpyMarioNrvDisable::sInstance);
        unpause();
    }
    void JumpyMario::exeEnable() {
        jumpy = true;
        pauseForSeconds(par::get("JumpOnSecs", 10));
        al::setNerve(this, &JumpyMarioNrvDisable::sInstance);
    }
    void JumpyMario::exeDisable() {
        jumpy = false;
        pauseForSeconds(par::get("JumpOffSecs", 10));
        al::setNerve(this, &JumpyMarioNrvEnable::sInstance);
    }
    void JumpyMario::deactivate() {
        Mod::deactivate();
        jumpy = false;
    }

} // namespace bm
