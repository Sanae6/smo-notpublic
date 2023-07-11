#pragma once

namespace al {
    class LiveActor;
}

struct PlayerPushReceiver {
    PlayerPushReceiver(const al::LiveActor*);
    void clear();
    void receivePushMsgHacker(al::SensorMsg const*, al::HitSensor const*, al::HitSensor const*,
                              float, bool);
    void calcPushedVelocity(sead::Vector3<float>*, sead::Vector3<float> const&) const;
private:
    char detail[0x40];
};
