#pragma once

#include "device.hpp"

/// @brief Air conditionor "realistic" as it modifies world temp gradually depending on power.
class RealAC : public Device {
public:
    RealAC(uint32_t power) : Device("RealAC"), k_power(power) {};

    void operate(std::shared_ptr<DeviceData> data) override;
    void malfunction(std::shared_ptr<DeviceData> data) override;
    uint32_t timeTravel(const uint32_t duration_sec) override;

private:
    /// @brief Assumption, a 1000w AC will cool or heat with rate 0.01 c/sec,
    /// which is 0.6 c/min or 3 Celsius degree after 5 mins. But since we simulate
    /// 1 min with 1 sec, it scales accordingly.
    /// What we need is deg per sec per watt, and watt is joule/sec, thus it's 0.6/1000
    static constexpr float K_DEG_PER_JOULE = 6e-4f;
    // max power in watt
    const uint32_t k_power;

    bool m_heat = false;
    Timer m_timer;

    /// @brief Our AC can operates in 100%, 50%, and 25% mode.
    /// Their values are also used to shift max power which is hundreds to thousands watts.
    enum class Mode : uint32_t {
        eFull = 0,
        eMid = 1,
        eLow = 2,
    } m_mode = Mode::eFull;

    bool setMode(std::string str);

    /// @brief Get actual power in watts modified by mode.
    inline float getPower() { return k_power >> static_cast<uint32_t>(m_mode); }

    /// @brief Async set AC open for certain mins.
    ///
    /// TODO: We spend 1 sec during execution on 1 min set by user. For now it is a implicit
    /// conversion. Later we will manage this properly in `SmartManager`.
    /// @param data `dfloat`, `dint`, `dbool`, `dstring` fields should store
    /// target temperature, duration (mins but actually executed in secs), heat or not, mode.
    void openForMins(std::shared_ptr<DeviceData> data);

    /// @brief Async set AC open till target degs.
    ///
    /// TODO: We spend 1 sec during execution on 1 min set by user. For now it is a implicit
    /// conversion. Later we will manage this properly in `SmartManager`.
    /// @param data `dfloat`, `dint`, `dbool`, `dstring` fields should store
    /// target temperature, duration (mins but actually executed in secs), heat or not, mode.
    void openTillDeg(std::shared_ptr<DeviceData> data);

    /// @brief Can be called at anytime after `openForMins()` and `openTillDeg()`.
    /// Besides updating temperature, it also stops the `Timer` if finished.
    void updateTemp();
};