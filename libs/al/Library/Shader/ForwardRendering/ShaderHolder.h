#pragma once

namespace al {
    class ShaderHolder {
        SEAD_SINGLETON_DISPOSER(ShaderHolder);

    public:
        void initAndLoadAllFromDir(const char*, sead::Heap*, sead::Heap*);
    };
} // namespace al