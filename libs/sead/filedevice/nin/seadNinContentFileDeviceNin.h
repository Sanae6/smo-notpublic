#pragma once

#include "filedevice/nin/seadNinFileDeviceBaseNin.h"

namespace sead
{
class NinContentFileDevice : public NinFileDeviceBase
{
    SEAD_RTTI_OVERRIDE_EXTERN(NinContentFileDevice, NinFileDeviceBase)

public:
    NinContentFileDevice();
};
}  // namespace sead
