#pragma once

#include <al/Library/Scene/Scene.h>

namespace seen {
    class CoolScene : public al::Scene {

    public:
        CoolScene(const char* name);
        void init(const al::SceneInitInfo &) override;
        void drawMain() override;

        void exeWait();
    };
} // namespace seen
