#pragma once

#include "PlayerCostumeInfo.h"
#include "PlayerModelHolder.h"
#include "game/Interfaces/IUseDimension.h"
#include <al/Library/LiveActor/LiveActor.h>

class PlayerPainPartsKeeper;

class PlayerModelChangerHakoniwa {
public:
    PlayerModelChangerHakoniwa(al::LiveActor const*, PlayerModelHolder*, PlayerPainPartsKeeper*, PlayerCostumeInfo*,
                               IUseDimension const*);
    void syncHost(bool);
    void update(bool alive, bool bound);
    virtual bool isFireFlower() const;
    virtual bool isMini() const;
    virtual bool is2DModel() const;
    virtual bool isHiddenModel() const;
    virtual bool isHiddenShadowMask() const;
    virtual void resetPosition();
    virtual void hideModel();
    virtual void hideSilhouette();
    virtual void hideShadowMask();
    virtual void showModel();
    virtual void showSilhouette();
    virtual void showShadowMask();
};
