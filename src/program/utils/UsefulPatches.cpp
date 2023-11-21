#include <utils/UsefulPatches.hpp>
#include <utils/Helpers.h>

namespace up {
    void usefulPatchesInit() {
        patch::CodePatcher patcher(0x1F2A2C);
        patcher.WriteInst(inst::Movz(W0, 0));
        patcher.Seek(0x1B3F0C);
        patcher.WriteInst(inst::Movz(W0, 0));
    }
} // namespace up
