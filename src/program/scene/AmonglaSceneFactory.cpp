#include <scene/CoolScene.hpp>
#include <scene/AmonglaSceneFactory.hpp>

namespace seen {
    template <typename T>
    al::Scene* createScene(const char* name) { return new T(name); }
    al::NameToCreator<alSceneFunction::CreationFunc> sceneFactoryFunctions[] = {
        {"CoolScene", createScene<CoolScene>},
    };
    AmonglaSceneFactory::AmonglaSceneFactory(const char* name) : alSceneFunction::SceneFactory(name) {
        initFactory(sceneFactoryFunctions);
    }
} // namespace seen
