#pragma once

#include <al/Library/LiveActor/LiveActor.h>
#include <al/Library/Nerve/Nerve.h>

namespace al {
    class WaterSurfaceFinder;
}
class CapTargetInfo;
class IUsePlayerHack;

class Pukupuku : public al::LiveActor {
    al::WaterSurfaceFinder* mWaterSurfaceFinder;
    CapTargetInfo* mCapTargetInfo;

public:
    IUsePlayerHack* mPlayerHack;
    explicit Pukupuku(const char* name);
    void endCapture();

    void exeCaptureStart();
    void exeCaptureWait();
};

class PukupukuNrvDoNothing : public al::Nerve {
    void execute(al::NerveKeeper* keeper) const override;

public:
    static PukupukuNrvDoNothing sInstance;
};
