#pragma once

#include <bonk/Mod.hpp>

namespace bm {
    class InputRandomizer : public Mod {
        void activate() override;
        void control() override;
    };
} // namespace bm
