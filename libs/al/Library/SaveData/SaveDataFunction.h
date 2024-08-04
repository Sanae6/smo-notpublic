#pragma once

#include <basis/seadTypes.h>

namespace al {
  u8* getSaveDataWorkBuffer();
  bool isInitializedSaveData();
  bool requestInitSaveDir(const char* fileName, u32 maxSize, u32 saveVersion);
  bool requestFormatSaveData(s32, s32);
  bool requestReadSaveData(const char* fileName, u32 maxSize, u32 saveVersion);
  bool requestWriteSaveData(const char* fileName, u32 maxSize, u32 saveVersion, bool);
  bool requestFlushSaveData();
  bool initSaveDirSync(const char* fileName, u32 size, u32 saveVersion);
  bool formatSaveDataSync(s32, s32);
  bool readSaveDataSync(const char* fileName, u32 maxSize, u32 saveVersion);
  bool writeSaveDataSync(const char* fileName, u32 maxSize, u32 saveVersion);
  void copyReadSaveDataFromBuffer(void* buffer, u32 bufferSize);
  void copyWriteSaveDataToBuffer(const void* buffer, u32 bufferSize);
  bool updateSaveDataSequence();
  bool isSuccessSaveDataSequence();
  bool isDoneSaveDataSequence();
  bool isCorruptedSaveDataSequenceRead();
  bool isCorruptedSaveDataSequenceResult();
  s32 getSaveDataSequenceResult();
  s32 getLastSaveDataFSErrorCode();
}  // namespace al