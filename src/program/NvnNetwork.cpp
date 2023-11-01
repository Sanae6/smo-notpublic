#include <NvnNetwork.hpp>
#include <al/Library/Base/String.h>
#include <hook/trampoline.hpp>
#include <nvn_FuncPtrBase.h>

namespace sus {
    bool nvnDeviceInitialize(NVNdevice* device, const NVNdeviceBuilder* builder) {
        
    }
}

struct ReplaceBootstrapLoader : public exl::TrampolineHook<ReplaceBootstrapLoader> {
    static void* Callback(const char* name) {
        if (al::isEqualString("nvnDeviceInitialize", name)) {
            
        }
    }
};

void nvnNetworkingInit() {}