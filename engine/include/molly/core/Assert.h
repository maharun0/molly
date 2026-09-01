#pragma once

#include "molly/core/Log.h"

#ifdef MOLLY_DEBUG
#if defined(_MSC_VER)
#define MOLLY_DEBUGBREAK() __debugbreak()
#else
#define MOLLY_DEBUGBREAK() ((void)0)
#endif

#define MOLLY_ASSERT(condition, ...)                    \
    do {                                                \
        if (!(condition)) {                              \
            MOLLY_CRITICAL("Assertion failed: {0}", __VA_ARGS__); \
            MOLLY_DEBUGBREAK();                           \
        }                                                 \
    } while (0)

#define MOLLY_ASSERT_UNUSED(condition) MOLLY_ASSERT(condition, #condition)
#else
#define MOLLY_ASSERT(condition, ...) ((void)0)
#define MOLLY_ASSERT_UNUSED(condition) ((void)(condition))
#endif
