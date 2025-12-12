#pragma once

#include "al/Library/LiveActor/LiveActor.h"
#include <span>

namespace cs {
  using CaptureSetup = void(*)();
  using CaptureHackStart = void(*)(al::LiveActor* actor, const sead::Vector3f& pos);
  using CaptureHackStarted = void(*)(al::LiveActor* actor);
  using CalcOffset = void(*)(sead::Vector3f& pos);
  struct CaptureInfo {
    const char* gameName;
    const char* hackName = nullptr;
    CaptureSetup setup = nullptr;
    CaptureHackStart hackStart = nullptr;
    CaptureHackStarted hackStarted = nullptr;
    sead::Vector3f offset = sead::Vector3f::zero;
    CalcOffset calcOffset = nullptr;

    static std::span<const CaptureInfo> getCaptures();
  };
} // namespace cs
