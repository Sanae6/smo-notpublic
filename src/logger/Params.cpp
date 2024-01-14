#include "Logger.hpp"
#include <al/Library/Base/String.h>
#include <basis/seadNew.h>
#include <diag/assert.hpp>
#include <heap/seadExpHeap.h>
#include <heap/seadHeapMgr.h>
#include <logger/Params.h>

Params Params::sInstance = {};
Params& Params::instance() { return sInstance; }
Params::Param& Params::
    getOrCreateParam(const char* name, const u8* value, size_t size) {
    EXL_ASSERT(params != nullptr, "Params is not yet initialized, wait until after the game starts!");
    for (int i = 0; i < length; i++)
        if (al::isEqualString(params[i].name, name))
            return params[i];

    auto& param = params[length++];
//    Logger::log("Created new param: %s %d %d\n", name, size, length - 1);
    auto nameLen = strlen(name);
    auto* nameAlloc = new (heap) char[nameLen + 1];
    nameAlloc[nameLen] = '\0';
    param.name = nameAlloc;
    strcpy(const_cast<char*>(param.name), name);
    param.data = new (heap) u8[size];
    memcpy(param.data, value, size);
    return param;
}
void Params::initialize() {
    heap = sead::ExpHeap::create(0x100000, "ParamHeap", sead::HeapMgr::sRootHeaps[0], 8,
                                 sead::Heap::cHeapDirection_Forward, true);
    params = new (heap) Param[0x100];
}
void Params::handleApply(ParamApplyPacket* packet) {
    auto& param = getOrCreateParam(packet->name(), packet->value(), packet->valueLen());
    if (param.size < packet->valueLen()) {
        EXL_ASSERT(param.data = static_cast<u8*>(heap->tryRealloc(param.data, packet->valueLen(), 8)), "Failed to reallocate memory for data");
        param.size = packet->valueLen();
    }
    memset(param.data, 0, param.size);
    memcpy(param.data, packet->value(), packet->valueLen());
//    Logger::log("Got parameter apply %s\n", packet->name());
}
void Params::handleApply(ParamDeletePacket* packet) {
    if (length == 0)
        return;
    Param* removedParam = nullptr;
    for (int i = 0; i < length; i++)
        if (al::isEqualString(params[i].name, packet->name)) {
            if (i != length - 1)
                std::swap(params[i], params[length - 1]);
            removedParam = &params[length - 1];
        }

    if (removedParam) {
        length--;
        heap->free(const_cast<char*>(params->name));
        heap->free(params->data);
    }
}
void Params::handleApply(TriggerPacket* packet) {
    bool value = true;
    Param& param = getOrCreateParam(packet->name, reinterpret_cast<const u8*>(&value), sizeof(bool));
    param.data[0] = true;
    Logger::log("Got trigger (game) %s\n", packet->name);
}
