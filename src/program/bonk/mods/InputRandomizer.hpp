#pragma once

#include <bonk/Mod.hpp>

namespace bm {
    struct InputRandomizer : public Mod {
        void activate() override;
        void control() override;
    };
} // namespace bm
