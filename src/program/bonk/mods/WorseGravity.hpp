#pragma once

#include <bonk/Mod.hpp>
namespace bm {
    struct WorseGravity : public ApplyMod{
        f32 originals[40];

        void deactivate() override;
        void apply() override;
        void control() override;
    };
} // namespace bm
