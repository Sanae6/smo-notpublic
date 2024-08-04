#pragma once

#include <al/Library/Nerve/NerveStateBase.h>
#include <game/HakoniwaSequence/HakoniwaSequence.h>

namespace cs {
  class CaptureSelectionState : public al::HostStateBase<HakoniwaSequence> {
    SEAD_SINGLETON_DISPOSER(CaptureSelectionState);
    CaptureSelectionState() = default;
    ~CaptureSelectionState() = default;
    void init() override;

public:
    void appear() override;
  };
} // namespace cs
