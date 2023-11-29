#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <bonk/mods/WideMario.hpp>

namespace bm {
    void WideMario::apply() {
        al::setScaleX(getMario(), 3);
        al::setScaleZ(getMario(), 3);
    }
    void WideMario::deactivate() {
        Mod::deactivate();
        if (inScene()) {
            al::setScaleX(getMario(), 1);
            al::setScaleZ(getMario(), 1);
        }
    }
} // namespace bm
