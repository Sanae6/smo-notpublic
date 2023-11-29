#pragma once

#include <bonk/Mod.hpp>

namespace bm {
    struct TallMario : public ApplyMod {
        void apply() override;
        void deactivate() override;
    };
} // namespace bm
