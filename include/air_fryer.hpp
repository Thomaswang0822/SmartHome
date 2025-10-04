#pragma once

#include "device.hpp"
#include "utils.hpp"

#include <assert.h>

class AirFryer : public Device {
public:
    /// @brief
    /// @param volume How big is the AirFryer in liter.
    AirFryer(float volume = 5.f)
        : Device("Air Fryer " + std::to_string(s_count++)), m_on(true), k_total_volume(volume),
          m_volume(volume) {};
    AirFryer(std::shared_ptr<DeviceData> data) : Device("Air Fryer") {
        if (data == nullptr) {
            AirFryer();
        } else {
            AirFryer(data->dfloat);
        }
    }
    ~AirFryer() { s_count--; }

    void Operate(std::shared_ptr<DeviceData> data) override;
    void Malfunction(std::shared_ptr<DeviceData> data) override;

private:
    // Data
    bool m_on = false;
    float k_total_volume;
    float m_volume;
    inline static uint32_t s_count = 0;

    // Functions
    void Cook(std::shared_ptr<DeviceData> data);
    void Cleanup(std::shared_ptr<DeviceData> data);
};
