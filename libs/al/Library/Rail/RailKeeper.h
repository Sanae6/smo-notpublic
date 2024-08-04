#pragma once

#include <al/Library/HostIO/HioNode.h>
#include <al/Library/Rail/IUseRail.h>

namespace al {
class RailRider;

class RailKeeper : public al::HioNode, public al::IUseRail {
private:
    al::RailRider* mRailRider{};

public:
    al::RailRider* getRailRider() const override { return mRailRider; }
};
};  // namespace al
