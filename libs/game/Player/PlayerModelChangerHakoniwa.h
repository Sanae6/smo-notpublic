#pragma once

#include "PlayerCostumeInfo.h"
#include "PlayerModelHolder.h"
#include "game/Interfaces/IUseDimension.h"
#include "IPlayerModelChanger.h"
#include <al/Library/LiveActor/LiveActor.h>

class PlayerPainPartsKeeper;

class PlayerModelChangerHakoniwa : public IPlayerModelChanger {
public:
    PlayerModelChangerHakoniwa(al::LiveActor const*, PlayerModelHolder*, PlayerPainPartsKeeper*, PlayerCostumeInfo*,
                               IUseDimension const*);
    void syncHost(bool);
    void update(bool alive, bool bound);
};
