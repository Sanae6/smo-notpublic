#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct Preprocessor : public ApplyMod {
        int filter = 1;
        void apply() override;
        void control() override;
    };
} // namespace bm
