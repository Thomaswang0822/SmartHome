#pragma once

#include "device_data.hpp"
#include "room.hpp"

#include <chrono>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

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

/// @brief Device base class. All devices in this project should extend from it.
/// It makes more sense for devices to affect `Room` directly, instead of the eaiser design:
/// `SmartManager` collects all updates and modifies the world.
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
    std::string getName() const { return m_name; }

    std::string getCurrentTime() const {
        return std::format("{:%T}", std::chrono::system_clock::now());
    }

    /// @brief Log what has been done in an `operate()` which is stored in `data->dstring`.
    /// @param data
    void logOperation(std::shared_ptr<DeviceData> data = nullptr) {
        if (data == nullptr) {
            std::cout << std::format("Empty log: I have done nothing at {}!\n", getCurrentTime());
        } else {
            std::cout << std::format(
                "{} log: {}\n", magic_enum::enum_name(data->op_id), data->dstring
            );
        }
    }

    /// @brief Should better be called before creating any Device instance.
    static void loginRoom(std::shared_ptr<Room> room) { s_room = room; }

    // BEGIN virtual functions

    /// @brief Simulate how the device behave when function properly
    /// @param op_id Identify which operations to be performed, because there can be many.
    virtual void operate(std::shared_ptr<DeviceData> data = nullptr) {
        if (data == nullptr || data->op_id == DeviceOpId::eDefault) {
            std::cout << "I am a " << this->getName() << " and I do NOTHING!" << std::endl;
        }
    }

    /// @brief Simulate time elapsing and update Device accordingly.
    /// It supports partial update for Device with relevant data.
    /// @example For `RealAC` that will open for 10s and go 25->20 degree, simulte for 6s will
    /// result in temperature updated to 22 degree.
    /// @example For `WasherDryer` that doesn't have "gradual" data like temperature, update is
    /// all or nothing except for the `Timer`.
    /// @param duration_sec If set to 0, the device should simulate till the finish of current
    /// opeation. Otherwise it simulate for exactly `duration_sec` seconds.
    /// @return How long we have simulated, equal to `duration_sec` if it != 0.
    virtual uint32_t timeTravel(const uint32_t duration_sec = 0) {
        std::this_thread::sleep_for(std::chrono::seconds(duration_sec));
        return duration_sec;
    }

    /// @brief Simulate how the device behave when function incorrectly
    /// @param mf_id Identify which operations to be performed, because there can be many.
    virtual void malfunction(std::shared_ptr<DeviceData> data = nullptr) {
        if (data == nullptr || data->mf_id == DeviceMfId::eNormal) {
            std::cerr << std::format("Philosophical question from {}: ", getName())
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
    /// @brief static because the room should be unique, while it's shared
    /// across all devices.
    inline static std::shared_ptr<Room> s_room = nullptr;

    /// @brief A universal malfunction corresponding to DeviceMfId::eHacked,
    /// replace the first `len` char of `m_name` with `newName`.
    /// `E.g. "DemoDevice_1".replace(0 /* from beginning */, 4, "Bad") = "BadDevice_1";`
    /// @param newName
    /// @param len
    void hackName(std::string newName, size_t len);
};

/// @brief A "better" placeholder class to demo
/// how an easiest-possible device works.
class DemoDevice : public Device {
public:
    DemoDevice(std::string name) : Device(name) {};
    /// @brief Operate() overridden by DemoDevice
    /// @param op_id Identify which operations to be performed, because there can be many.
    void operate(std::shared_ptr<DeviceData> data) override;

    void malfunction(std::shared_ptr<DeviceData> data) override;

private:
    // All normal operations
    std::string hello() const;
    std::string sing() const;

    // All malfunctions

    // Special data
};
