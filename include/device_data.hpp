#pragma once

#include "magic_enum/magic_enum.hpp"

#include <iostream>
#include <string>

enum class OpId : uint32_t {
    eDefault = 0,
    // for DemoDevice
    eHello = 1,
    eSing = 2,
    // for AirFryer
    eAirFryerCook = 3,
    eAirFryerClean = 4,
    // for WashDryer
    eWashDryerCombo = 5,
    eWashDryerWashOnly = 6,
    eWashDryerDryOnly = 7,
    // for RealAC
    eRealAcOpenTillDeg = 8,
    eRealAcOpenForMins = 9,

    COUNT,
};

/// @brief Universal Device operation data.
struct DeviceDataBase {
    // specific things
    bool success = false;
    std::string log_str;
    OpId op_id;
    // malfunction id common for all devices.
    enum class MfId : uint32_t {
        eNormal = 0,
        eLowBattery = 1,
        eHacked = 2,
        eBroken = 3,

        COUNT,
    } mf_id;

    inline void logOpId() const {
        std::cout << "Because of unrecognized OpId::" << magic_enum::enum_name(op_id) << std::endl;
    }
    inline void logMfId() const {
        std::cout << "Because of unrecognized MfId::" << magic_enum::enum_name(mf_id) << std::endl;
    }

    // Not stri, but we need a virtual function to turn on vtable
    virtual std::string getDataType() const { return typeid(this).name(); };
};

struct AirFryerData : DeviceDataBase {
    float food_volume;
    uint32_t time_sec;
};

struct RealACData : DeviceDataBase {
    float target_temp;
    int duration_sec;
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
    // a natural design for both having same volume, i.e you dry what you just washed.
    float cloth_volume;
    uint32_t wash_sec;
    uint32_t dry_sec;
};
