#pragma once

#include <cassert>
#include <format>

#define DEBUG_ASSERT(condition, ...)                                                               \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            std::fprintf(stderr, "%s\n", std::format(__VA_ARGS__).c_str());                        \
            std::fflush(stderr);                                                                   \
            assert(false && #condition);                                                           \
        }                                                                                          \
    } while (false)