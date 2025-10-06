#pragma once

#include "magic_enum/magic_enum.hpp"
#include <iostream>
#include <string>

enum class DeviceOpId : uint32_t {
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

    COUNT,
};

enum class DeviceMfId : uint32_t {
    eNormal = 0,
    eLowBattery = 1,
    eHacked = 2,
    eBroken = 3,

    COUNT,
};

/// @brief Data struct to unify input & output of ALL devices.
struct DeviceData {
    float dfloat;
    int dint;
    bool dbool;
    std::string dstring;
    // specific things
    bool success = false;
    DeviceOpId op_id;
    DeviceMfId mf_id;

    inline void LogOpId() const {
        std::cout << "Because of unrecognized DeviceOpId::" << magic_enum::enum_name(op_id)
                  << std::endl;
    }
    inline void LogMfId() const {
        std::cout << "Because of unrecognized DeviceMfId::" << magic_enum::enum_name(mf_id)
                  << std::endl;
    }
};