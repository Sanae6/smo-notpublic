#include "logger/Logger.hpp"
#include "prim/seadRuntimeTypeInfo.h"
#include <filedevice/seadFileDevice.h>
#include <prim/seadSafeString.h>
#include <resource/seadArchiveRes.h>
#include <resource/seadResource.h>
#include <resource/seadResourceMgr.h>

namespace fd {
  class FolderArchiveRes : public sead::ArchiveRes {
  bool checkDerivedRuntimeTypeInfo(const sead::RuntimeTypeInfo::Interface*) const override { return true; }
  const sead::RuntimeTypeInfo::Interface* getRuntimeTypeInfo() const override { return nullptr; }

public:
    FolderArchiveRes(const sead::SafeString& path, sead::FileDevice* device);
    ~FolderArchiveRes() override;

    const void* getFileImpl_(const sead::SafeString& file_path,
                             sead::ArchiveRes::FileInfo* file_info = NULL) const override;
    const void* getFileFastImpl_(s32 entry_id, sead::ArchiveRes::FileInfo* file_info) const override { Logger::log("kys 0\n"); return nullptr; }
    s32 convertPathToEntryIDImpl_(const sead::SafeString& file_path) const override { Logger::log("kys 1\n"); return 0; }
    bool setCurrentDirectoryImpl_(const sead::SafeString&) override { Logger::log("kys 2\n"); return false; }
    bool openDirectoryImpl_(sead::HandleBuffer* handle, const sead::SafeString& path) const override { Logger::log("kys 3\n"); return false; }
    bool closeDirectoryImpl_(sead::HandleBuffer* handle) const override { Logger::log("kys 4\n"); return false; }
    u32 readDirectoryImpl_(sead::HandleBuffer* handle, sead::DirectoryEntry* entry, u32 num) const override { Logger::log("kys 5\n"); return false; }
    bool isExistFileImpl_(const sead::SafeString& path) const override { Logger::log("kys 6\n"); return false; }
    bool prepareArchive_(const void* archive) override { Logger::log("kys 7\n"); return true; }

    sead::FixedSafeString<512> pathBase;
    sead::FileDevice* device;
  };

  class FolderArchiveFactory : public sead::DirectResourceFactoryBase {
    SEAD_RTTI_OVERRIDE(FolderArchiveFactory, sead::DirectResourceFactoryBase)

public:
    sead::FileDevice* usedFileDevice;
    sead::ResourceFactory* parent;
    FolderArchiveFactory(sead::FileDevice* usedFileDevice, sead::ResourceFactory* parent)
        : usedFileDevice(usedFileDevice), parent(parent) {}

    sead::Resource* tryCreateDirectory(const sead::ResourceMgr::LoadArg& loadArg);
    sead::Resource* tryCreate(const sead::ResourceMgr::LoadArg& loadArg) override;
    sead::Resource* tryCreateWithDecomp(const sead::ResourceMgr::LoadArg& loadArg,
                                        sead::Decompressor* decompressor) override;
    sead::Resource* create(const sead::ResourceMgr::CreateArg& createArg) override;

    sead::DirectResource* newResource_(sead::Heap* heap, s32 alignment) override { return nullptr; }
  };
} // namespace fd
