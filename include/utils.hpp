#pragma once

#include "magic_enum/magic_enum.hpp"

#include <cassert>
#include <chrono>
#include <format>

namespace Debug {

template <typename... Args>
inline void logAssert(bool condition, std::format_string<Args...> fmt, Args&&... args) {
    if (!condition) [[unlikely]] {
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        std::fprintf(stderr, "Assertion failed: %s\n", message.c_str());
        std::fflush(stderr);
        assert(false);
    }
}

/// @brief `enum class` can contains invalid values, thus we need to throw in the
/// `default` branch of a switch on an enum.
/// @tparam EnumType one of the enum type defined in device_data.hpp
template <typename EnumType>
class DeviceOperationException : public std::runtime_error {
public:
    DeviceOperationException(const EnumType enum_value)
        : std::runtime_error(
              std::format(
                  "Invalid value {} for enum {}", magic_enum::enum_name(enum_value),
                  typeid(enum_value).name()
              )
          ) {}
};
} // namespace Debug

/// @brief Timer a reusable time check that does NOT simulate time elapsing.
struct Timer {
    Timer() = default;
    void begin(uint32_t total_time_sec) {
        t_total_sec = std::chrono::seconds(total_time_sec);
        t_start = std::chrono::system_clock::now();
        running = true;
    }

    /// @brief Check if the timer is still running.
    /// If time is up (now - start >= target), `Stop()` and return 0;
    /// Else, return remaining time.
    /// @endcond If return 0, the Timer is ready to use.
    ///
    /// Note that `running` bool check should be done outside.
    /// @return remaining_time Output, unit is sec.
    int checkRemainingTime() {
        using namespace std::chrono;
        if (!running) {
            // just a safe guard.
            return 0;
        } else if (duration_cast<seconds>(system_clock::now() - t_start) >= t_total_sec) {
            stop();
            return 0;
        } else {
            // can use .count() directly since we use consistent unit second.
            return (t_total_sec - duration_cast<seconds>(system_clock::now() - t_start)).count();
        }
    }

    /// @brief set to not running state
    void stop() { running = false; }

    std::chrono::system_clock::time_point t_start;
    std::chrono::seconds t_total_sec;
    bool running = false;
};
