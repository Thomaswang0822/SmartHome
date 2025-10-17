#pragma once

#include "magic_enum/magic_enum.hpp"

#include <iostream>
#include <string>
#include <variant>

/// @brief Universal Device operation data.
struct DeviceDataBase {
    // specific things
    bool success = false;
    // std::string log_str;
    // malfunction id common for all devices.
    enum class MfId : uint32_t {
        eNormal = 0,
        eLowBattery = 1,
        eHacked = 2,
        eBroken = 3,
    };

    // Not necessary, but we need a virtual function to turn on vtable
    virtual std::string getDataType() const { return typeid(this).name(); };
};

struct DemoDeviceData : DeviceDataBase {
    enum class OpId : uint32_t {
        eHello = 0,
        eSing = 1,
    };
    std::variant<MfId, OpId> id = MfId::eNormal; // Default to normal;
};

struct AirFryerData : DeviceDataBase {
    enum class OpId : uint32_t {
        eAirFryerCook = 0,
        eAirFryerClean = 1,
    };
    std::variant<MfId, OpId> id = MfId::eNormal; // Default to normal;

    float food_volume;
    uint32_t time_sec;
};

struct RealACData : DeviceDataBase {
    enum class OpId : uint32_t {
        eRealAcOpenTillDeg = 0,
        eRealAcOpenForMins = 1,
    };
    std::variant<MfId, OpId> id = MfId::eNormal; // Default to normal;

    float target_temp;
    uint32_t duration_sec;
    bool is_heat;

    /// @brief Our AC can operates in 100%, 50%, and 25% mode.
    /// Their values are also used to shift max power which is hundreds to thousands watts.
    enum class Mode : uint32_t {
        eFull = 0,
        eMid = 1,
        eLow = 2,
    } mode = Mode::eFull;
};

struct WasherDryerData : DeviceDataBase {
    enum class OpId : uint32_t {
        eWashDryerCombo = 0,
        eWashDryerWashOnly = 1,
        eWashDryerDryOnly = 2,
    };
    std::variant<MfId, OpId> id = MfId::eNormal; // Default to normal;

    // a natural design for both having same volume, i.e you dry what you just washed.
    float cloth_volume;
    uint32_t wash_sec;
    uint32_t dry_sec;
    // user can choose hack name
    std::string hack_name;
};
