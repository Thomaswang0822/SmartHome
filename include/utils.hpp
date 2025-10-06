#pragma once

#include <cassert>
#include <format>

namespace Debug {

template <typename... Args>
inline void Assert(bool condition, std::format_string<Args...> fmt, Args&&... args) {
    if (!condition) [[unlikely]] {
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        std::fprintf(stderr, "Assertion failed: %s\n", message.c_str());
        std::fflush(stderr);
        assert(false);
    }
}

} // namespace Debug