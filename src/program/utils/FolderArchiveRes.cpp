#include <utils/FolderArchiveFactory.hpp>

namespace fd {
  FolderArchiveRes::FolderArchiveRes(const sead::SafeString& path, sead::FileDevice* device)
      : sead::ArchiveRes(), pathBase(path), device(device) {}
  FolderArchiveRes::~FolderArchiveRes() = default;
  const void* FolderArchiveRes::getFileImpl_(const sead::SafeString& file_path,
                                             sead::ArchiveRes::FileInfo* file_info) const {
    sead::FormatFixedSafeString<512> subPath("%s/%s", pathBase.cstr(), file_path.cstr());

    auto loadArg = sead::FileDevice::LoadArg{.path = subPath};
    auto data = device->tryLoad(loadArg);
    if (file_info) {
      file_info->mStartOffset = 0;
      file_info->mLength = loadArg.read_size;
    }

    return data;
  }
} // namespace fd
