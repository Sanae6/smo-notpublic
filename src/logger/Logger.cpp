#include "Logger.hpp"
#include "Packets.h"
#include "SocketInterface.h"
#include "util.h"

static void logBase(SocketInterface& interface, const char* fmt, va_list args) {
    u16 length = nn::util::VSNPrintf(nullptr, 0, fmt, args);
    if (length == 0) return;

    auto* packet = (LogPacket*)alloca(sizeof(LogPacket) + length + 2);
    packet->type = PacketType::Log;
    packet->size = length + 2;
    length = nn::util::VSNPrintf(packet->buffer, packet->size, fmt, args);
    packet->buffer[length] = '\0';

    interface.send(packet, 0);
}

void Logger::log(const char *fmt, ...) {
    auto& interface = SocketInterface::instance();
    if (!interface.isConnected()) return;

    va_list args;
    va_start(args, fmt);
    logBase(interface, fmt, args);
    va_end(args);
}

void Logger::log(const char *fmt, va_list args) {
    auto& interface = SocketInterface::instance();
    if (!interface.isConnected()) return;

    logBase(interface, fmt, args);
}