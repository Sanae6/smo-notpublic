#pragma once
#include <utils/ForwardDecls.hpp>

namespace sp {

  class StaticManager {
    al::LayoutActor* overScreen;
    al::NoiseTextureKeeper* noiseTextureKeeper;
    nn::ui2d::TextureInfo* textureInfo;
    nvn::Sampler sampler;
    MoviePlayer* moviePlayer;

public:
    static void init();
    static void draw();
    StaticManager(const al::ActorInitInfo& initInfo);
    ~StaticManager();

    void update() const;
  };

} // namespace sp
