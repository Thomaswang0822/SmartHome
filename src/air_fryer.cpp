#include "air_fryer.hpp"

#include <thread>

void AirFryer::implOperate(std::shared_ptr<AirFryerData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->op_id) {
    case OpId::eAirFryerCook:
        cook(data);
        break;
    case OpId::eAirFryerClean:
        cleanup(data);
        break;
    default:
        Device::operate();
        data->logOpId();
        break;
    }
}

void AirFryer::implMalfunction(std::shared_ptr<AirFryerData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->mf_id) {
    case DeviceDataBase::MfId::eLowBattery:
        m_on = false;
        break;
    case DeviceDataBase::MfId::eHacked:
        // replace "Air" with "Evil"
        hackName("Evil", 3);
        break;
    case DeviceDataBase::MfId::eBroken:
        std::cerr << getName() << " is buring! BOOM! Buy a new one!" << std::endl;
        break;
    default:
        // eNormal
        Device::malfunction();
        break;
    }
}

uint32_t AirFryer::implTimeTravel(const uint32_t duration_sec) {
    // cook is sync (blocking sleep_for), thus no updated needed also
    std::this_thread::sleep_for(std::chrono::seconds(duration_sec));
    return duration_sec;
}

void AirFryer::cook(std::shared_ptr<AirFryerData> data) {
    // caller Operate() should filter out nullptr input
    Debug::logAssert(data != nullptr, "caller Operate() should filter out nullptr input");
    Debug::logAssert(
        data->food_volume > 0.f, "Food Volume shoud be positive, got %.3f", data->food_volume
    );

    if (data->food_volume > k_total_volume) {
        data->log_str = std::format(
            "Food volume {} bigger than total volume {}. Buy a bigger one!",
            data->food_volume,
            k_total_volume
        );
        data->success = false;
    } else if (data->food_volume > m_volume) {
        data->log_str = std::format(
            "Food volume {} bigger than current volume {}. Wait for more space.",
            data->food_volume,
            m_volume
        );
        data->success = false;
    }
    m_volume -= data->food_volume;
    std::this_thread::sleep_for(std::chrono::seconds(data->time_sec));
    data->log_str =
        std::format("completes cooking after {} seconds at {}", data->time_sec, getCurrentTime());
    data->success = true;
}

void AirFryer::cleanup(std::shared_ptr<AirFryerData> data) {
    m_volume = k_total_volume;
    data->success = true;
    data->log_str = std::format("cleanup done at {}", getCurrentTime());
}
