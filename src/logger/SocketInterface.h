#pragma once

#include "Packets.h"
#include <basis/seadTypes.h>
#include <os.h>
#include <result.h>
#include <socket.hpp>

class SocketInterface {
    static SocketInterface sInstance;

    s32 fd{};
    in_addr hostAddress{};
    sockaddr serverAddress{};
    bool startedUp{};
    bool connected{};
    nn::os::ThreadType thread{};
    nn::os::LightEventType connectedEvent{};
    nn::os::LightEventType initializedEvent{};

    static void threadEntrypoint(void*);
    void threadMain();
    static void handlePacket(Packet* packet) ;
    template <typename T>
    bool read(T* value) const {
        return read(value, sizeof(T));
    }
    bool read(void* data, u64 length) const;

public:
    bool init(const char* ip, u16 port);

    void send(const Packet* packet, s32 flags = 0) const;
    void send(const void* data, u64 length, s32 flags = 0) const;

    void signalInit();
    void waitForConnection();

    bool isConnected() const { return connected; }
    static SocketInterface& instance() { return sInstance; }
};
