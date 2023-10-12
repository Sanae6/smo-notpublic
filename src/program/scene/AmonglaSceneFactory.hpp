#pragma once

#include <al/Library/Scene/SceneFactory.h>

namespace seen {
    class AmonglaSceneFactory : public alSceneFunction::SceneFactory{
    public:
        AmonglaSceneFactory(const char* name);
    };
} // namespace seen
