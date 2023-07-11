#pragma once
/**
 * @file PlayerActorBase.h
 * @brief Interfaces for Classes that use PlayerHack (PlayerActorBase)
* Vtable loc:
 */

class PlayerHackKeeper;

class IUsePlayerHack {
public:
    virtual PlayerHackKeeper* getPlayerHackKeeper() const = 0;
};
