#pragma once

#include "filedevice/nin/seadNinFileDeviceBaseNin.h"

namespace sead
{
class NinHostIOFileDevice : public NinFileDeviceBase
{
    SEAD_RTTI_OVERRIDE_EXTERN(NinHostIOFileDevice, NinFileDeviceBase)

public:
    NinHostIOFileDevice();

    bool doIsAvailable_() const override;
    bool formatPathForFS_(BufferedSafeString* out, const SafeString& path) const override;
};
}  // namespace sead
