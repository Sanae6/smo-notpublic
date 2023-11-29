#include <al/Library/Memory/HeapUtil.h>
#include <logger/Logger.hpp>
#include <utils/Helpers.h>
#include <utils/UsefulPatches.hpp>

namespace al {
    class Resource;
    class ResourceSystem {
    public:
        class ResourceCategory;
    };
} // namespace al
namespace up {
    struct ResourceHeapTracking : Trampoline<ResourceHeapTracking> {
        static al::Resource* Callback(al::ResourceSystem* resourceSystem, sead::SafeString& string,
                                      al::ResourceSystem::ResourceCategory* category, char* suffix) {
            auto heap = al::getCurrentHeap();
            if (al::getCurrentHeap() == nullptr) heap = unsafeRef<sead::Heap*>(category, 0x98);
            auto heapName = heap->getName();
            Logger::log("Before create %s %s %s %d\n", heapName.cstr(), string.cstr(), suffix ?: "NULL", heap->getFreeSize());
            auto res = Orig(resourceSystem, string, category, suffix);
            Logger::log("After create %s %s %s %d\n", heapName.cstr(), string.cstr(), suffix ?: "NULL", heap->getFreeSize());
            return res;
        }
    };
    void usefulPatchesInit() {
        patch::CodePatcher patcher(0x1F2A2C);
        patcher.WriteInst(inst::Movz(W0, 1));
        patcher.Seek(0x1B3F0C);
        patcher.WriteInst(inst::Nop());
//        ResourceHeapTracking::InstallAtSymbol(
//            "_ZN2al14ResourceSystem14createResourceERKN4sead14SafeStringBaseIcEEPNS0_16ResourceCategoryEPKc");
    }
} // namespace up
