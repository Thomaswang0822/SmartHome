#include "air_fryer.hpp"

#include <thread>

void AirFryer::Operate(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    uint32_t time_ms = static_cast<uint32_t>(data->dint);
    float food_volume = data->dfloat;
    DEBUG_ASSERT(food_volume > 0.f, "Food Volume shoud be positive, got %.3f", food_volume);

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

    std::cout << data->dstring << std::endl;
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
        HackName("Evil");
        break;
    case DeviceMfId::eBroken:
        std::cout << GetName() << " is broken!" << std::endl;
        break;
    default:
        // eNormal
        Device::Malfunction();
        break;
    }
}

void AirFryer::Cook(std::shared_ptr<DeviceData> data) {
    // caller Operate() should filter out nullptr input
    DEBUG_ASSERT(data != nullptr, "caller Operate() should filter out nullptr input");
    auto food_volume = data->dfloat;
    auto time_ms = data->dint;

    if (food_volume > k_total_volume) {
        data->dstring = std::format(
            "Food volume {} bigger than {} total volume {}. Buy a bigger one!",
            food_volume,
            GetName(),
            k_total_volume
        );
        data->success = false;
    } else if (food_volume > m_volume) {
        data->dstring = std::format(
            "Food volume {} bigger than {} current volume {}. Wait for more space.",
            food_volume,
            GetName(),
            m_volume
        );
        data->success = false;
    }
    m_volume -= food_volume;
    std::this_thread::sleep_for(std::chrono::milliseconds(time_ms));
    data->dstring = std::format("{} completes cooking after {} miliseconds", GetName(), time_ms);
    data->success = true;
}

void AirFryer::Cleanup(std::shared_ptr<DeviceData> data) {
    m_volume = k_total_volume;
    data->success = true;
    data->dstring = std::format("{} cleanup done", GetName());
}
