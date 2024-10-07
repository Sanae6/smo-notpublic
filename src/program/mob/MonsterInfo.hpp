#pragma once

#include "al/Library/LiveActor/LiveActor.h"
#include <span>

namespace mob {
  using MonsterSetup = void (*)();
  using MonsterStart = void (*)(al::LiveActor* actor, const al::ActorInitInfo& initInfo, const sead::Vector3f& pos);
  using MonsterStarted = void (*)(al::LiveActor* actor, const al::ActorInitInfo& initInfo, const sead::Vector3f& pos);
  using CalcOffset = void (*)(sead::Vector3f& pos);
  using MonsterRetarget = al::LiveActor* (*)(al::LiveActor* actor);
  struct MonsterInfo {
    const char* gameName;
    MonsterSetup setup = nullptr;
    MonsterStart start = nullptr;
    MonsterStarted started = nullptr;
    sead::Vector3f offset = sead::Vector3f::zero;
    CalcOffset calcOffset = nullptr;
    MonsterRetarget retarget = nullptr;

    static std::span<const MonsterInfo> getMonsters();
  };
} // namespace mob