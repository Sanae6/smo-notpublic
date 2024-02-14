#pragma once

#include "al/Library/LiveActor/LiveActor.h"
#include "game/Player/PlayerActorHakoniwa.h"
namespace sp {
  class SePlayObjSpooky : public al::LiveActor {
    PlayerActorHakoniwa* player;
    public:
    SePlayObjSpooky(PlayerActorHakoniwa* player) : al::LiveActor("SePlayObjSpooky"), player(player) {}

    void init(const al::ActorInitInfo &info) override;

    void playSound(const char* sound);
  };
}
