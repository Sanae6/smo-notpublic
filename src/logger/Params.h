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
    static Param staticParamBuffer[];

    sead::Heap* heap = nullptr;

    Param* params = nullptr;
    u64 length = 0;
    Params::Param& getOrCreateParam(const char* name, const u8* value, size_t valueSize);
public:
    static Params& instance();
    static bool isInitialized() {
        return instance().heap != nullptr;
    }
    void initialize();
    void handleApply(ParamApplyPacket* packet);
    void handleApply(ParamDeletePacket* packet);
    template <typename T>
    T get(const char* name, T defValue) {
        return *reinterpret_cast<T*>(getOrCreateParam(name, reinterpret_cast<u8*>(&defValue), sizeof(T)).data);
    }
    const char* getString(const char* name, const char* defValue) {
        return reinterpret_cast<const char*>(getOrCreateParam(name, reinterpret_cast<const u8*>(defValue), strlen(defValue) + 1).data);
    }
};

namespace par {
    [[maybe_unused]] static const char* get(const char* name, const char* defValue) {
        if (!Params::isInitialized()) return defValue;
        return Params::instance().getString(name, defValue);
    }

    template <typename T>
    [[maybe_unused]] static T get(const char* name, const T defValue) {
        if (!Params::isInitialized()) return defValue;
        return Params::instance().get(name, defValue);
    }
}