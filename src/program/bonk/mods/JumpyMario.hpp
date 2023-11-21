#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct JumpyMario : ApplyMod {
        JumpyMario();
        void apply() override;
    };
} // namespace bm
