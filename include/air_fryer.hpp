#pragma once

#include "device.hpp"
#include "utils.hpp"

#include <assert.h>

/// @brief TODO: Add "AirFryer Concurrency":
/// 0  min: add fish that takes 20 mins;
/// 5  min: add chicken wings that takes 10 mins;
/// 15 min: chicken wings ready;
/// 20 min: fish ready;
class AirFryer : public Device {
public:
    /// @brief
    /// @param volume How big is the AirFryer in liter.
    AirFryer(float volume = 5.f) : Device("AirFryer"), k_total_volume(volume), m_volume(volume) {};
    /// @brief
    /// @param data Should store `k_total_volume = m_volume` in `data->dfloat`
    AirFryer(std::shared_ptr<DeviceData> data)
        : Device("Air Fryer"), k_total_volume(data->dfloat), m_volume(data->dfloat) {}
    ~AirFryer() = default;

    void operate(std::shared_ptr<DeviceData> data) override;
    void malfunction(std::shared_ptr<DeviceData> data) override;

private:
    // Data
    const float k_total_volume;
    float m_volume;

    // Functions
    void cook(std::shared_ptr<DeviceData> data);
    void cleanup(std::shared_ptr<DeviceData> data);
};
