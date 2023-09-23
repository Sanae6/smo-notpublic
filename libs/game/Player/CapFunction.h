#pragma once

#include "HackCap.h"
#include "PlayerAnimator.h"

class CapFunction
{
    public:
        static void putOnCapPlayer(HackCap *, PlayerAnimator *);
        static void endHack(HackCap*, PlayerAnimator*);
};
