#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <bonk/mods/TallMario.hpp>

namespace bm {
    void TallMario::apply() {
        al::setScaleY(getMario(), 3);
    }
} // namespace bm
