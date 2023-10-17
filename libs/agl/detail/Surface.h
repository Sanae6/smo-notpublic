/**
 * @file Surface.h
 * @brief Defines classes to setup NVN textures on a surface.
 */

#pragma once

#include "types.h"
#include "agl/util.h"
#include "nvn.h"

namespace agl
{
    namespace detail
    {
        struct SurfaceBase;

        class Surface
        {
        public:
            Surface();
            void initialize(agl::TextureType, agl::TextureFormat, u32, agl::TextureAttribute, agl::MultiSampleType);
            void initializeSize(u32, u32, u32);
            void copyFrom(agl::detail::SurfaceBase const &);
            void calcSizeAndAlignment();
            void setupNVNtextureBuilder(NVNtextureBuilder *) const;
            void printInfo() const;
            void copyFrom(NVNtexture const &);

            u16 mWidth;
            u16 mHeight;
            u16 depth;
            u16 driverFormat;
            u8 multiSampleType;
            u8 levels;
            u16 textureType;
            u32 storageAlignment;
            u32 storageSize;
            u32 unk0;
            u16 storageClass;
            u8 textureAttribute;
            u8 pixelByteSize;
            u32 stride;
            u8 compSel[4];
        };

        struct SurfaceBase
        {
            u64 _0;
            u64 _8;
            u64 _10;
        };
    };
};