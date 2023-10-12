#pragma once

#include <al/Library/Nerve/NerveExecutor.h>
#include <al/Library/LiveActor/ActorInitInfo.h>

namespace bm {
    struct Mod {
        bool active = false;
        virtual void init(const al::ActorInitInfo& initInfo) {}
        virtual void activate() { active = true; }
        void update() {
            if (active)
                control();
        }

    protected:
        virtual void control(){};
    };

    struct NerveMod : public Mod, public al::NerveExecutor {
    protected:
        NerveMod() : al::NerveExecutor("Mod with NerveExecutor") {}
        void control() override {
            updateNerve();
        }
    };
} // namespace bm
