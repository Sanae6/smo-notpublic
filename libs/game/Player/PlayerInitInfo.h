#pragma once

#include "al/Library/LiveActor/ActorInitInfo.h"

class PlayerInitInfo {
  public:
  al::GamePadSystem* GamePadSystem;
  sead::Matrix34f* viewMatrix;
  s32 port;
  bool idfk;
  const char* costume;
  const char* cap;
  sead::Vector3f position;
  sead::Quatf rotation;
  bool noseNeedle;
  bool isClosetMario;
};

static_assert(sizeof(PlayerInitInfo) == 0x48);
