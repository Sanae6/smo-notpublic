#include <al/Library/Yaml/Writer/ByamlWriter.h>

class ByamlSave {
public:
    virtual void write(al::ByamlWriter) = 0;
    virtual void read(const al::ByamlIter&) = 0;
};

class PlayerDamageKeeper : public ByamlSave {
private:
    bool mIsKidsMode;
    s32 mHitPoint;
    bool mHasLifeUpMax;
    bool mForceNormalMode;
public:

};