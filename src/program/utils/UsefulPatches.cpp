#include "filedevice/nin/seadNinFileDeviceBaseNin.h"
#include "filedevice/seadFileDeviceMgr.h"
#include "filedevice/seadPath.h"
#include "resource/seadArchiveRes.h"
#include "resource/seadResourceMgr.h"
#include <al/Library/Memory/HeapUtil.h>
#include <logger/Logger.hpp>
#include <utils/FolderArchiveFactory.hpp>
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
  class SdFolderDevice : public sead::NinFileDeviceBase {
    SEAD_RTTI_OVERRIDE(SdFolderDevice, NinFileDeviceBase)

public:
    SdFolderDevice() : sead::NinFileDeviceBase("sd", "sd") {}

    bool doIsAvailable_() const override { return sead::FileDeviceMgr::instance()->hasMountedSd(); }

    bool formatPathForFS_(sead::BufferedSafeString* out, const sead::SafeString& path) const override {
      out->format("%s:/smo/%s", mMountPoint.cstr(), path.cstr());
      sead::Path::changeDelimiter(out, '/');
      return true;
    }
  };

  struct ResourceHeapTracking : Trampoline<ResourceHeapTracking> {
    static al::Resource* Callback(al::ResourceSystem* resourceSystem, sead::SafeString& string,
                                  al::ResourceSystem::ResourceCategory* category, char* suffix) {
      auto heap = al::getCurrentHeap();
      if (al::getCurrentHeap() == nullptr)
        heap = unsafeRef<sead::Heap*>(category, 0x98);
      auto heapName = heap->getName();
      Logger::log("Before create %s %s %s %d\n", heapName.cstr(), string.cstr(), suffix ?: "NULL", heap->getFreeSize());
      auto res = Orig(resourceSystem, string, category, suffix);
      Logger::log("After create %s %s %s %d\n", heapName.cstr(), string.cstr(), suffix ?: "NULL", heap->getFreeSize());
      return res;
    }
  };
  struct SetupFolderArchiveFactory : Trampoline<SetupFolderArchiveFactory> {
    static void Callback(class SystemKit* sysKit, const char* sysFilename, s32 parallelInt, s32 memorySize,
                         bool runOnFirstCore) {
      Orig(sysKit, sysFilename, parallelInt, memorySize, runOnFirstCore);
      auto mgr = sead::ResourceMgr::instance();
      auto parent = mgr->findFactory("sarc");
      mgr->unregisterFactory(parent);
      auto factory = alloc<fd::FolderArchiveFactory>(alloc<SdFolderDevice>(), parent);
      mgr->registerFactory(factory, "sarc");
    }
  };
  void usefulPatchesInit() {
    patch::CodePatcher patcher(0x1F2A2C);
    patcher.WriteInst(inst::Movz(W0, 1));
    patcher.Seek(0x1B3F0C);
    patcher.WriteInst(inst::Nop());
    //        ResourceHeapTracking::InstallAtSymbol(
    //            "_ZN2al14ResourceSystem14createResourceERKN4sead14SafeStringBaseIcEEPNS0_16ResourceCategoryEPKc");
    SetupFolderArchiveFactory::InstallAtSymbol("_ZN2al9SystemKit20createResourceSystemEPKciib");
  }
} // namespace up
