#include "air_fryer.hpp"

#include <thread>

void AirFryer::Operate(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->op_id) {
    case DeviceOpId::eAirFryerCook:
        this->Cook(data);
        break;
    case DeviceOpId::eAirFryerClean:
        this->Cleanup(data);
        break;
    default:
        Device::Operate();
        data->LogOpId();
        break;
    }
}

void AirFryer::Malfunction(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->mf_id) {
    case DeviceMfId::eLowBattery:
        m_on = false;
        break;
    case DeviceMfId::eHacked:
        // replace "Air" with "Evil"
        HackName("Evil", 3);
        break;
    case DeviceMfId::eBroken:
        std::cout << GetName() << " is buring! BOOM! Buy a new one!" << std::endl;
        break;
    default:
        // eNormal
        Device::Malfunction();
        break;
    }
}

void AirFryer::Cook(std::shared_ptr<DeviceData> data) {
    // caller Operate() should filter out nullptr input
    Debug::Assert(data != nullptr, "caller Operate() should filter out nullptr input");
    float food_volume = data->dfloat;
    Debug::Assert(food_volume > 0.f, "Food Volume shoud be positive, got %.3f", food_volume);
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

void AirFryer::Cleanup(std::shared_ptr<DeviceData> data) {
    m_volume = k_total_volume;
    data->success = true;
    data->dstring = std::format("cleanup done");
}
