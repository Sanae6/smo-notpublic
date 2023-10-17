#pragma once

#include "PlayerCostumeInfo.h"
#include "PlayerModelHolder.h"
#include "game/Interfaces/IUseDimension.h"
#include "IPlayerModelChanger.h"
#include <al/Library/LiveActor/LiveActor.h>

class PlayerPainPartsKeeper;

class PlayerModelChangerHakoniwa : public IPlayerModelChanger {
private:
    al::LiveActor* actor;
    bool isChanged;
    bool is2D;
    al::LiveActor* currentModel;
    PlayerModelHolder* playerModelHolder;
    PlayerPainPartsKeeper* playerPainPartsKeeper;
    PlayerCostumeInfo* playerCostumeInfo;
    IUseDimension* dimension;
    bool isDirty;
    bool showingModel;
    bool showingSilhouette;
    bool showingShadowMask;
    bool isDamageStopDemo;
    int damageStopDemoCountdown;
    bool handledSituation;
    bool hideHairVisibility;
    bool syncBodyHair;
    bool syncFaceBeard;
    bool syncHeadStrap;

public:
    PlayerModelChangerHakoniwa(al::LiveActor const*, PlayerModelHolder*, PlayerPainPartsKeeper*, PlayerCostumeInfo*,
                               IUseDimension const*);
    void syncHost(bool);
    void update(bool alive, bool bound);
};
