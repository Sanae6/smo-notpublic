#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <bonk/mods/TallMario.hpp>

namespace bm {
    void TallMario::apply() { al::setScaleY(getMario(), 3); }
    void TallMario::deactivate() {
        Mod::deactivate();
        if (inScene())
            al::setScaleY(getMario(), 1);
    }
} // namespace bm
