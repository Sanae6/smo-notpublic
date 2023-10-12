#pragma once

#include <al/Library/Scene/Scene.h>
#include <al/Library/Factory/Factory.h>

namespace alSceneFunction {
    using CreationFunc = al::Scene*(*)(const char* name);
    class SceneFactory : public al::Factory<CreationFunc> {
    public:
        SceneFactory(const char* name) : al::Factory<CreationFunc>(name) {}
    };
}