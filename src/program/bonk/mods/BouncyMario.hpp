#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct BouncyMario : public Mod {
        float multiplier = 1.0f;
        void control() override;
    };
} // namespace bm
