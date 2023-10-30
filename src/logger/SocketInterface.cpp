#include "SocketInterface.h"
#include "Logger.hpp"
#include "Params.h"
#include <lib.hpp>
#include <nifm.h>
#include <socket.hpp>
#include <utils/Helpers.h>

static char socketPool[0x400000 + 0x20000] __attribute__((aligned(0x1000)));
static u8 threadStack[0x8000] __attribute__((aligned(0x1000)));
static u8 packetBuffer[0x10000];

SocketInterface SocketInterface::sInstance;


struct SocketInterfaceInitialization : exl::hook::impl::TrampolineHook<SocketInterfaceInitialization> {
    static void Callback(void* initArg) {
        Orig(initArg);
        Params::instance().initialize();
        auto& interface = SocketInterface::instance();
        interface.signalInit();
//        interface.waitForConnection();
    }
};

bool SocketInterface::init(const char* ip, u16 port) {
    SocketInterfaceInitialization::InstallAtSymbol("_ZN4sead9Framework10initializeERKNS0_13InitializeArgE");
    nn::os::InitializeLightEvent(&initializedEvent, false, nn::os::EventClearMode::Manual);

    nn::nifm::Initialize();

    nn::socket::Initialize(socketPool, 0x400000, 0x20000, 0xE);

    nn::nifm::SubmitNetworkRequest();

    while (nn::nifm::IsNetworkRequestOnHold()) {
    }

    if (!nn::nifm::IsNetworkAvailable())
        return false;

    if ((fd = nn::socket::Socket(2, 1, 0)) < 0)
        return false;

    nn::os::InitializeLightEvent(&connectedEvent, false, nn::os::EventClearMode::Manual);
    nn::os::SignalLightEvent(&connectedEvent);

    nn::socket::InetAton(ip, &hostAddress);
    serverAddress.address = hostAddress;
    serverAddress.port = nn::socket::InetHtons(port);
    serverAddress.family = 2;

    nn::os::CreateThread(&thread, threadEntrypoint, nullptr, threadStack, sizeof(threadStack), 19);
    nn::os::SetThreadName(&thread, "SocketInterface");
    nn::os::StartThread(&thread);

    nn::os::WaitLightEvent(&connectedEvent);

    startedUp = true;

    return true;
}
void SocketInterface::send(const Packet* packet, s32 flags) const {
    nn::socket::Send(fd, packet, packet->size + 4, flags);
}
void SocketInterface::send(const void* data, u64 length, s32 flags) const { nn::socket::Send(fd, data, length, flags); }
void SocketInterface::waitForConnection() { nn::os::WaitLightEvent(&connectedEvent); }
void SocketInterface::threadEntrypoint(void*) {
    instance().threadMain();
}
bool SocketInterface::read(void* data, u64 length) const {
    u8* dataPtr = static_cast<u8*>(data);
    u64 dataLen = length;
    while (dataLen > 0) {
        s32 readLen = nn::socket::Recv(fd, dataPtr, dataLen, 0);
        if (readLen <= 0)
            return false;
        dataLen -= readLen;
        dataPtr += readLen;
    }
    return true;
}
void SocketInterface::threadMain() {
    nn::os::SignalLightEvent(&connectedEvent);
    void* data = packetBuffer;
    Packet* packet = static_cast<Packet*>(data);

    while (true) {
        nn::os::WaitLightEvent(&initializedEvent);
        s32 result = nn::socket::Connect(fd, &serverAddress, sizeof(serverAddress));
        if (result < 0) {
            svcSleepThread(3000000000);
            continue;
        }

        connected = true;
        nn::os::SignalLightEvent(&connectedEvent);
        Logger::log("Connected!\n");

        while (true) {
            if (!read(packet)) {
                Logger::log("failed? %d\n", nn::socket::GetLastErrno());
                break;
            }

            if (!read(packet->data(), packet->size)) {
                Logger::log("failed?? %d\n", nn::socket::GetLastErrno());
                break;
            }

            handlePacket(packet);
        }
        connected = false;
        nn::os::ClearLightEvent(&connectedEvent);
        svcSleepThread(5000000000);
    }
}

void SocketInterface::handlePacket(Packet* packet) {
    switch (packet->type) {
    case PacketType::Log:
        break;
    case PacketType::ParamApply:
        Params::instance().handleApply(reinterpret_cast<ParamApplyPacket*>(packet));
        break;
    case PacketType::ParamDelete:
        Params::instance().handleApply(reinterpret_cast<ParamDeletePacket*>(packet));
        break;
    case PacketType::Trigger:
        Params::instance().handleApply(reinterpret_cast<TriggerPacket*>(packet));
        break;
    }
}
void SocketInterface::signalInit() { nn::os::SignalLightEvent(&initializedEvent); }
