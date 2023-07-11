#pragma once

namespace al {
    class LiveActor;
    class HitSensor {
    public:
        const char* name;
        char _data[0x38];
        al::LiveActor* host;
    };
} // namespace al
