#pragma once

#include "device.hpp"

#include <deque>

/// @brief A FIFO async Washer-Dryer twin.
/// Unlike AirFryer, WashDryer should act atomically:
/// User should NOT be able to add or take out cloth in the middle.
///
/// "Async" in the sense that `Operate()` only submits the wash/dry job and returns immediately.
/// `FinishAll()` should be called to really execute all waiting cloth in the bin when properly.
class WasherDryer : public Device {
public:
    WasherDryer(float volume = 5.f) : Device("WasherDryer"), k_total_volume(volume) {}
    WasherDryer(std::shared_ptr<DeviceData> data)
        : Device("WasherDryer"), k_total_volume(data->dfloat) {}
    ~WasherDryer() = default;

    void operate(std::shared_ptr<DeviceData> data) override;
    void malfunction(std::shared_ptr<DeviceData> data) override;
    uint32_t timeTravel(const uint32_t duration_sec) override;

private:
    // a natural design for both having same volume
    const float k_total_volume;
    Timer m_wash_timer = {};
    Timer m_dry_timer = {};
    std::deque<std::shared_ptr<DeviceData>> m_wash_bin;
    std::deque<std::shared_ptr<DeviceData>> m_dry_bin;

    /// @brief Async Wash operation.
    /// 1. add input wash data to bin.
    /// 2. [if washer occupied now] sim running wash till the end and finish up.
    /// 3. [if wash just finished is a wash-dry combo] submit to dryer.
    /// 4. submit input wash.
    /// @param data NOTE: `data->success` stores result of itself, not of finished previous wash.
    void wash(std::shared_ptr<DeviceData> data);

    /// @brief Async Dry operation, works the same as `Wash()` except for step 3.
    void dry(std::shared_ptr<DeviceData> data);

    void performNext(bool is_wash);
};