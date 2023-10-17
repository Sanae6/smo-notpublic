#include <bonk/mods/StickInverter.hpp>

namespace bm {
    void invert(sead::Vector2f& out) { out *= -1.0f; }

    void StickInverter::activate() {
        Mod::activate();
        patch::CodePatcher patcher(0x861968);
        patcher.BranchInst((void*)&invert);
        patcher.Seek(0x8618d4);
        patcher.BranchInst((void*)&invert);
    }
} // namespace bm
