#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <bonk/mods/WideMario.hpp>

namespace bm {
    void WideMario::apply() {
        al::setScaleX(getMario(), 3);
        al::setScaleZ(getMario(), 3);
    }
} // namespace bm
