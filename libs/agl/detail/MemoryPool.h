/**
 * @file MemoryPool.h
 * @brief Defines classes for memory pools for storage.
 */

#pragma once

#include "types.h"
#include <nvn/nvn.h>

namespace agl
{
    enum class MemoryAttribute {};

    namespace detail
    {
        class MemoryPoolType
        {
        public:
            static s32 convert(agl::MemoryAttribute);

            static u32 cInvalidPoolType; // 0
            static u32 cValidPoolType; // 0x80000000
        };

        class MemoryPool
        {
        public:
            MemoryPool();

            void initialize(void *, u64, agl::detail::MemoryPoolType const &);
            void initialize(void *, u64, agl::detail::MemoryPoolType const &, agl::detail::MemoryPool const &, s32);

            void finalize();

            NVNmemoryPool pool;
            s32 memoryPoolType;
        };
    };
};