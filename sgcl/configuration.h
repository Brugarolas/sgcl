//------------------------------------------------------------------------------
// SGCL: Smart Garbage Collection Library
// Copyright (c) 2022-2024 Sebastian Nibisz
// SPDX-License-Identifier: Zlib
//------------------------------------------------------------------------------
#pragma once

//#define SGCL_DEBUG

// the maximum sleep time of the GC thread in seconds
#define SGCL_MAX_SLEEP_TIME_SEC 30
// the percentage amount of allocations that will wake up the GC thread
#define SGCL_TRIGER_PERCENTAGE 25

#ifdef SGCL_DEBUG
#define SGCL_LOG_PRINT_LEVEL 3
#endif

#include "metadata_base.h"
#include <cstddef>

namespace sgcl {
    struct metadata : metadata_base {
        virtual void to_string(void*) {}
    };

    namespace Priv {
        static constexpr ptrdiff_t MaxStackSize = 0x100000;
        static constexpr ptrdiff_t StackDetectionOffset = 1024;
        static constexpr size_t MaxTypeNumber = 4096;
        static constexpr int DeletionDelayMsec = 100;
   }
}
