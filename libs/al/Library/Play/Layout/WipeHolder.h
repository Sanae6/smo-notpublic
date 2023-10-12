#pragma once

#include <basis/seadTypes.h>

namespace al {
    class WipeSimple;

    class WipeHolder {
        struct WipePair {

        };
    private:
        s32 mWipeCount;
        s32 mAddedWipes;
        s32 mCurrentWipeIndex;
        WipePair* mWipePair;
        bool mClosed;
        WipePair* mWipeInfo;
        WipePair* mActiveWipe;
        s32 mWipeInfoCount;

    public:
        WipeHolder(s32);
        void registerWipe(const char* wipeName, al::WipeSimple*);
        void startClose(const char* wipeName, s32);
        void findWipe(const char* wipeName) const;
        void startCloseByInfo(const char* wipeName);
        void findInfo(const char* wipeName) const;
        bool tryStartClose(const char* wipeName, s32);
        bool tryStartCloseByInfo(const char* wipeName);
        void startCloseEnd(const char* wipeName);
        void startOpen(s32);
        void isExistInfo(const char* wipeName) const;
        bool tryFindInfo(const char* wipeName) const;
        s32 getCloseTimeByInfo(const char* wipeName) const;
        bool isCloseEnd() const;
        bool isOpenEnd() const;
        bool isCloseWipe(const char* wipeName) const;
    };
} // namespace al