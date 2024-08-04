#pragma once

#include <game/MapObj/ChangeStageInfo.h>

#include "al/Library/System/GameDataHolderBase.h"
#include "basis/seadTypes.h"
#include "game/GameData/GameDataFile.h"
#include "game/WorldList/WorldList.h"

class GameDataHolder : public al::GameDataHolderBase,
                       public al::ISceneObj,
                       public al::HioNode,
                       public al::IUseMessageSystem {
private:
    al::MessageSystem* mMessageSystem;
    GameDataFile** mDataFiles;
    GameDataFile* mDataFile;
    GameDataFile* mNextDataFile;
    u32 mGameFileIndex;
    class SaveDataAccessSequence* mSaveDataAccessSequence;
    bool mRequireSave;
    u32 mRequireSaveFrame;
    bool mIsInvalidSaveForMoonGet;
    bool mChangeStageRelated;
    bool mStageEnded;
    sead::FixedSafeString<32> mLanguage; // _50
    u64 mGlobalPlayTime;
    sead::Heap* mByamlIterHeap;
    u8* mSaveFileBuffer;
    class GameConfigData* mGameConfigData;
    class TempSaveData* mTempSaveData[2];
    class CapMessageBossData* capMessageBossData;
    u8 unknown0[0x18];
    bool* startedScenarioCameras;
    sead::PtrArray<void> mStageLocks; // moon requirements, see GameDataHolder::GameDataHolder for struct
    sead::PtrArray<void> mItemList;
    sead::PtrArray<void> mItemListE3;
    sead::PtrArray<void> mItemClothList;
    sead::PtrArray<void> mItemCapList;
    sead::PtrArray<void> mItemGiftList;
    sead::PtrArray<void> mItemStickerList;
    sead::PtrArray<void> mHackObjInfos;
    sead::PtrArray<void> mUnlockedItems;
    u8 field_170[0x10]; // block of unknown data
    class AchievementInfoReader* mAchievementInfoReader;
    class AchievementHolder* mAchievementHolder;
    WorldList* mWorldList; // 0x190
    sead::PtrArray<void> mChangeStageList;
    sead::PtrArray<void> mExStageList;
    sead::PtrArray<void> mInvalidOpenMapList;
    sead::PtrArray<void> unknown2;
    bool* mShouldShowBindTutorial;
    class MapDataHolder* mMapDataHolder;
    sead::PtrArray<void> mWorldItemTypeList;
    void* mCoinCollectNum;
    s32* mWorldWarpLinks;
    void** mWorldWarpHoleInfos;
    s32 mWorldWarpHoleInfoCount;
    UniqObjInfo* mUniqObjInfo;
    bool field_220;
    s32 field_224;
    bool field_228;
    s32 field_22C;
    s32 field_230;
    s32 field_234;
    sead::Vector3f* mCoinTransForDeadPlayer;
    s32 mCoinTransForDeadPlayerCount;
    bool field_244;
    bool mIsSeparatePlay;
    bool field_246;
    class QuestInfoHolder* mQuestInfoHolder;
    bool field_250;
    class GameSequenceInfo* mGameSequenceInfo;
    void* field_260;

public:
    GameDataHolder(const al::MessageSystem*);
    ~GameDataHolder() override;

    const char* getSceneObjName() override;
    const al::MessageSystem* getMessageSystem() const override;

    void setPlayingFileId(s32 file);
    void intitalizeData();
    void initialzeDataCommon();
    void resetTempSaveData(bool);
    void initializeDataId(s32);
    void readByamlData(s32, char const*);
    s32 tryFindEmptyFileId() const;

    bool isRequireSave() const;
    void setRequireSave();
    void setRequireSaveFalse();
    void setRequireSaveFrame();
    void updateRequireSaveFrame();
    bool isInvalidSaveForMoonGet() const;
    void invalidateSaveForMoonGet();
    void validateSaveForMoonGet();
    void setLanguage(char const*);
    char* getLanguage() const;

    void resetLocationName();
    void changeNextStageWithDemoWorldWarp(char const*);
    bool tryChangeNextStageWithWorldWarpHole(char const*);
    void returnPrevStage();
    char* getNextStageName() const;
    char* getNextStageName(s32 idx) const;
    GameDataFile* getGameDataFile(s32 idx) const;
    // u64 getNextPlayerStartId() const;
    char* getCurrentStageName() const;
    char* tryGetCurrentStageName() const;
    char* getCurrentStageName(s32 idx) const;
    // void setCheckpointId(al::PlacementId const *);
    char* tryGetRestartPointIdString() const;
    void endStage();
    void startStage(char const*, s32);
    // void onObjNoWriteSaveData(al::PlacementId const *);
    // void offObjNoWriteSaveData(al::PlacementId const *);
    // bool isOnObjNoWriteSaveData(al::PlacementId const *) const;
    // void onObjNoWriteSaveDataResetMiniGame(al::PlacementId const*);
    // void offObjNoWriteSaveDataResetMiniGame(al::PlacementId const *);
    // bool isOnObjNoWriteSaveDataResetMiniGame(al::PlacementId const *) const;
    // void onObjNoWriteSaveDataInSameScenario(al::PlacementId const *);
    // bool isOnObjNoWriteSaveDataInSameScenario(al::PlacementId const *) const;
    void writeTempSaveDataToHash(char const*, bool);

    void resetMiniGameData();
    s32 getPlayingFileId() const;

    s32 findUnlockShineNum(bool*, s32) const;
    s32 calcBeforePhaseWorldNumMax(s32) const;
    bool isFindKoopaNext(s32) const;
    bool isBossAttackedHomeNext(s32) const;
    void playScenarioStartCamera(s32);
    bool isPlayAlreadyScenarioStartCamera() const;

    s32 getShineAnimFrame(s32) const;
    s32 getCoinCollectNumMax(s32) const;

    void readFromSaveDataBufferCommonFileOnlyLanguage();
    void readFromSaveDataBuffer(const char* bufferName);
    void writeToSaveDataBuffer(const char* name);

    void changeNextStage(ChangeStageInfo const*, int);

    int findUseScenarioNo(char const*);
};