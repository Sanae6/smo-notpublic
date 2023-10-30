#pragma once

#include "Packets.h"
#include <heap/seadHeap.h>

class Params {
    static Params sInstance;
    struct Param {
        const char* name;
        u8* data;
        size_t size;
    };

    sead::Heap* heap;

    Param* params;
    u64 length = 0;
    nn::os::MutexType mutex;
    Param& getOrCreateParam(const char* name, const u8* value, size_t valueSize);
public:
    static Params& instance();
    void initialize();
    void handleApply(ParamApplyPacket* packet);
    void handleApply(ParamDeletePacket* packet);
    void handleApply(TriggerPacket* packet);
    template <typename T>
    T get(const char* name, T defValue) {
        return *reinterpret_cast<T*>(getOrCreateParam(name, reinterpret_cast<u8*>(&defValue), sizeof(T)).data);
    }
    const char* getString(const char* name, const char* defValue) {
        return reinterpret_cast<const char*>(getOrCreateParam(name, reinterpret_cast<const u8*>(defValue), strlen(defValue) + 1).data);
    }
    bool clicked(const char* name, bool clear) {
        bool value = false;
        Param& p = getOrCreateParam(name, reinterpret_cast<const u8*>(&value), sizeof(bool));
        value = p.data[0] > 0;
        if (value && clear)
            p.data[0] = false;
        return value;
    }
};

namespace par {
    [[maybe_unused]] static const char* get(const char* name, const char* defValue) {
        return Params::instance().getString(name, defValue);
    }

    template <typename T>
    [[maybe_unused]] static T get(const char* name, const T defValue) {
        return Params::instance().get(name, defValue);
    }

    [[maybe_unused]] static bool clicked(const char* name, bool clear = true) {
        return Params::instance().clicked(name, clear);
    }
}