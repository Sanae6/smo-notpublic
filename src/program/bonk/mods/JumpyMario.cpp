#include <bonk/mods/JumpyMario.hpp>

namespace bm {
    static bool jumpy = false;
    struct JumpCodeAlwaysActive : Trampoline<JumpCodeAlwaysActive> {
        static bool Callback(const IUsePlayerCollision* collision) {
            if (jumpy) return true;
            return Orig(collision);
        }
    };
    JumpyMario::JumpyMario() {
        JumpCodeAlwaysActive::InstallAtSymbol("_ZN2rs19isCollisionCodeJumpEPK19IUsePlayerCollision");
    }
    void JumpyMario::apply() { jumpy = true; }
} // namespace bm
