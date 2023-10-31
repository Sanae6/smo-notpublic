#include <bonk/mods/CameraLock.hpp>

namespace bm {
    static al::CameraTargetBase* cameraTarget = nullptr;
    struct TargetHook : Trampoline<TargetHook> {
        static al::CameraTargetBase* Callback(al::CameraTargetHolder* holder, int index) {
            return cameraTarget ?: Orig(holder, index);
        }
    };
    namespace {
        MAKE_NERVE_BOTH(CameraLock, Lock);
        MAKE_NERVE_BOTH(CameraLock, Unlocked);
    }
    void CameraLock::activate() { Mod::activate();
        TargetHook::InstallAtSymbol("_ZNK2al18CameraTargetHolder13getViewTargetEi");
        initNerve(&CameraLockNrvLock::sInstance, 0);
    }
    void CameraLock::exeLock() {
        sceneInfo.mCameraDirector->mCameraTargetHolder->getViewTarget(0)->calcTrans(&target.lastTrans);
        cameraTarget = &target;
        pauseForSeconds(par::get("CamLockedSecs", 10));
        al::setNerve(this, &CameraLockNrvUnlocked::sInstance);
    }
    void CameraLock::exeUnlocked() {
        cameraTarget = nullptr;
        pauseForSeconds(par::get("CamUnlockedSecs", 10));
        al::setNerve(this, &CameraLockNrvLock::sInstance);
    }
} // namespace bm
