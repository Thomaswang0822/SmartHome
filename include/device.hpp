#pragma once

#include "device_data.hpp"

#include <chrono>
#include <format>
#include <iostream>
#include <memory>
#include <string>

/// @brief Timer a reusable time check that does NOT simulate time elapsing.
struct Timer {
    Timer() = default;
    void Begin(uint32_t total_time_sec) {
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
    int CheckRemainingTime() {
        using namespace std::chrono;
        if (!running) {
            // just a safe guard.
            return 0;
        } else if (duration_cast<seconds>(system_clock::now() - t_start) >= t_total_sec) {
            Stop();
            return 0;
        } else {
            // can use .count() directly since we use consistent unit second.
            return (t_total_sec - duration_cast<seconds>(system_clock::now() - t_start)).count();
        }
    }

    /// @brief Clear and set to uninitialized state, NOT pause.
    void Stop() {
        t_total_sec = std::chrono::seconds(0);
        running = false;
    }

    std::chrono::system_clock::time_point t_start;
    std::chrono::seconds t_total_sec;
    bool running = false;
};

/// @brief Device base class. All devices in this project should extend from it.
class Device {
public:
    /// @brief Constructor
    /// @param name device name, should be unique.
    Device(std::string name) : m_name(name + "_" + std::to_string(s_global_id)), m_on(true) {
        s_total_count++;
        s_global_id++;
    }

    /// @brief
    /// @return device name
    std::string GetName() const { return m_name; }

    void LogOperation(std::shared_ptr<DeviceData> data = nullptr) {
        if (data == nullptr) {
            std::cout << std::format("Empty log by {}: I have done nothing!.\n", GetName());
        } else {
            std::cout << std::format(
                "{} log by {}: {}\n", magic_enum::enum_name(data->op_id), GetName(), data->dstring
            );
        }
    }

    // BEGIN virtual functions

    /// @brief Simulate how the device behave when function properly
    /// @param op_id Identify which operations to be performed, because there can be many.
    virtual void Operate(std::shared_ptr<DeviceData> data = nullptr) {
        if (data == nullptr || data->op_id == DeviceOpId::eDefault) {
            std::cout << "I am a " << this->GetName() << " and I do NOTHING!" << std::endl;
        }
    }

    /// @brief Simulate how the device behave when function incorrectly
    /// @param mf_id Identify which operations to be performed, because there can be many.
    virtual void Malfunction(std::shared_ptr<DeviceData> data = nullptr) {
        if (data == nullptr || data->mf_id == DeviceMfId::eNormal) {
            std::cerr << std::format("Philosophical question from {}: ", GetName())
                      << "If I run normally while malfunction, do I run correctly or incorrectly?"
                      << std::endl;
        }
    }

    virtual ~Device() { s_total_count--; }

protected:
    std::string m_name = "NULL";
    bool m_on = false;
    // increment only
    inline static uint32_t s_global_id = 0;
    // increment & decrement
    inline static uint32_t s_total_count = 0;

    /// @brief A universal malfunction corresponding to DeviceMfId::eHacked,
    /// replace the first `len` char of `m_name` with `newName`.
    /// `E.g. "DemoDevice_1".replace(0 /* from beginning */, 4, "Bad") = "BadDevice_1";`
    /// @param newName
    /// @param len
    void HackName(std::string newName, size_t len);
};

/// @brief A "better" placeholder class to demo
/// how an easiest-possible device works.
class DemoDevice : public Device {
public:
    DemoDevice(std::string name) : Device(name) {};
    /// @brief Operate() overridden by DemoDevice
    /// @param op_id Identify which operations to be performed, because there can be many.
    void Operate(std::shared_ptr<DeviceData> data) override;

    void Malfunction(std::shared_ptr<DeviceData> data) override;

private:
    // All normal operations
    std::string Hello() const;
    std::string Sing() const;

    // All malfunctions

    // Special data
};
