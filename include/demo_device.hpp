#pragma once

#include "device.hpp"
#include "device_data.hpp"

/// @brief A "better" placeholder class to demo
/// how an easiest-possible device works.
class DemoDevice : public Device<DemoDevice, DeviceDataBase> {
public:
    DemoDevice(std::string name) : Device(name) {};
    /// @brief Operate() overridden by DemoDevice
    /// @param op_id Identify which operations to be performed, because there can be many.
    void implOperate(std::shared_ptr<DeviceDataBase> data);

    void implMalfunction(std::shared_ptr<DeviceDataBase> data);

    uint32_t implTimeTravel(const uint32_t duration_sec) {
        // nothing timed here
        std::this_thread::sleep_for(std::chrono::seconds(duration_sec));
        return duration_sec;
    }

private:
    // All normal operations
    std::string hello() const;
    std::string sing() const;

    // Special data
};