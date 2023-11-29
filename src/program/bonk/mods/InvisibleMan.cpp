#include <al/Library/LiveActor/ActorModelFunction.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <bonk/mods/InvisibleMan.hpp>

namespace bm {
    void farAway(al::LiveActor* model) {
        al::setTrans(model, -100000.0f * sead::Vector3f::ey);
    }

    void InvisibleMan::activate() {
        Mod::activate();

        updateAlways = true;
        patch::CodePatcher patcher(0x45e1f4); // disable model on load
        patcher.BranchLinkInst((void*)&farAway);
    }

    void InvisibleMan::deactivate() {
        patch::CodePatcher patcher(0x45e1f4); // disable model on load
        patcher.BranchLinkInst((void*)&al::copyPose);
    }
} // namespace bm
