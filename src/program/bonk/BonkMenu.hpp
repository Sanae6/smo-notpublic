#pragma once

#include <al/Library/Nerve/NerveExecutor.h>
namespace bm {
    struct BonkMenu : al::NerveExecutor {
        static BonkMenu* instance;
        const char* text = nullptr;
        float alpha = 0;
        BonkMenu();
        void appear(const char* text);
        void control();
        void exeAppear();
        void exeWait();
        void exeHide();
    };
} // namespace bm
