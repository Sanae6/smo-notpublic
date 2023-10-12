#pragma once

namespace alSceneFunction {
    class SceneFactory;
}
namespace al {
    class IUseSceneCreator;
    struct SequenceInitInfo;
    class GameDataHolderBase;
    class ScreenCaptureExecutor;
    void initSceneCreator(al::IUseSceneCreator*, const al::SequenceInitInfo&, al::GameDataHolderBase*,
                          al::AudioDirector*, al::ScreenCaptureExecutor*, alSceneFunction::SceneFactory*);
    void initAudioDirector(al::Sequence*, al::AudioSystemInfo*, al::AudioDirectorInitInfo&);
    void setSceneAndUseInitThread(al::IUseSceneCreator*, al::Scene*, int, const char*, int, const char*, sead::Heap*);
    bool tryEndSceneInitThread(al::IUseSceneCreator*);
} // namespace al