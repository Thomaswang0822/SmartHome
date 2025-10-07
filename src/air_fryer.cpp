#include "air_fryer.hpp"

#include <thread>

void AirFryer::operate(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->op_id) {
    case DeviceOpId::eAirFryerCook:
        cook(data);
        break;
    case DeviceOpId::eAirFryerClean:
        cleanup(data);
        break;
    default:
        Device::operate();
        data->logOpId();
        break;
    }
}

void AirFryer::malfunction(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->mf_id) {
    case DeviceMfId::eLowBattery:
        m_on = false;
        break;
    case DeviceMfId::eHacked:
        // replace "Air" with "Evil"
        hackName("Evil", 3);
        break;
    case DeviceMfId::eBroken:
        std::cout << getName() << " is buring! BOOM! Buy a new one!" << std::endl;
        break;
    default:
        // eNormal
        Device::malfunction();
        break;
    }
}

void AirFryer::cook(std::shared_ptr<DeviceData> data) {
    // caller Operate() should filter out nullptr input
    Debug::logAssert(data != nullptr, "caller Operate() should filter out nullptr input");
    float food_volume = data->dfloat;
    Debug::logAssert(food_volume > 0.f, "Food Volume shoud be positive, got %.3f", food_volume);
    auto time_sec = data->dint;

    if (food_volume > k_total_volume) {
        data->dstring = std::format(
            "Food volume {} bigger than total volume {}. Buy a bigger one!",
            food_volume,
            k_total_volume
        );
        data->success = false;
    } else if (food_volume > m_volume) {
        data->dstring = std::format(
            "Food volume {} bigger than current volume {}. Wait for more space.",
            food_volume,
            m_volume
        );
        data->success = false;
    }
    m_volume -= food_volume;
    std::this_thread::sleep_for(std::chrono::seconds(time_sec));
    data->dstring = std::format("completes cooking after {} seconds", time_sec);
    data->success = true;
}

void AirFryer::cleanup(std::shared_ptr<DeviceData> data) {
    m_volume = k_total_volume;
    data->success = true;
    data->dstring = std::format("cleanup done");
}
