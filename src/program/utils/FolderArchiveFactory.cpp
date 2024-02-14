#include "program/utils/FolderArchiveFactory.hpp"
#include "al/Library/Memory/HeapUtil.h"
#include "filedevice/seadFileDevice.h"
#include "logger/Logger.hpp"
#include "prim/seadSafeString.h"
#include "resource/seadResource.h"
#include "utils/Helpers.h"
namespace fd {
  sead::Resource* FolderArchiveFactory::tryCreateDirectory(const sead::ResourceMgr::LoadArg& loadArg) {    
    sead::FixedSafeString<512> path(loadArg.path);
    if (path.endsWith(".szs")) {
      path.chop(4);
    }
    if (usedFileDevice->isExistDirectory(path)) {
      Logger::log("Using folder archive %s\n", path.cstr());
      FolderArchiveRes* res = allocWithHeap<FolderArchiveRes>(
        loadArg.instance_heap,
        loadArg.instance_alignment ? loadArg.instance_alignment : 8,
        path,
        usedFileDevice
      );
      return res;
    }
    return nullptr;
  }

  sead::Resource* FolderArchiveFactory::tryCreate(const sead::ResourceMgr::LoadArg& loadArg) {
    auto res = tryCreateDirectory(loadArg);
    return res ? res : parent->tryCreate(loadArg);
  }
  sead::Resource* FolderArchiveFactory::tryCreateWithDecomp(const sead::ResourceMgr::LoadArg& loadArg,
                                                            sead::Decompressor* decompressor) {
    auto res = tryCreateDirectory(loadArg);
    return res ? res : parent->tryCreateWithDecomp(loadArg, decompressor);
  }
  sead::Resource* FolderArchiveFactory::create(const sead::ResourceMgr::CreateArg& createArg) {
    return parent->create(createArg);
  }
} // namespace fd
