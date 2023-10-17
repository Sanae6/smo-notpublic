#pragma once

#include <basis/seadTypes.h>

enum class PacketType : u16 {
    Log,
    ParamApply,
    ParamDelete,
    Trigger,
};

struct Packet {
    PacketType type;
    u16 size;
    Packet() = default;
    Packet(PacketType type, u16 size) : type(type), size(size) {}
    void* data() const {
        return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + sizeof(Packet));
    }
    template <typename T>
    T& as() {
        return *reinterpret_cast<T>(this);
    }
};

struct LogPacket : Packet {
    explicit LogPacket(u16 size) : Packet(PacketType::Log, size) {}
    char buffer[];
};

struct ParamApplyPacket : Packet {
    u16 nameLen;
    const u8 heap[];

    const char* name() const {
        return reinterpret_cast<const char*>(heap);
    }

    const u8* value() const {
        return heap + nameLen;
    }

    u16 valueLen() const {
        return size - nameLen;
    }
};

struct ParamDeletePacket : Packet {
    const char name[];
};
