#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct NoOxygen : public Mod {
        void activate() override;
    };
} // namespace bm
