#include "air_fryer.hpp"

#include <thread>

void AirFryer::implOperate(std::shared_ptr<AirFryerData> data) {
    if (data == nullptr || !m_on)
        return;

    auto op_id = getOpId(data);
    switch (op_id) {
        using enum AirFryerData::OpId;
    case eAirFryerCook:
        cook(data);
        break;
    case eAirFryerClean:
        cleanup(data);
        break;
    default:
        // shouldn't reach here
        throw Debug::DeviceOperationException<AirFryerData::OpId>(op_id);
        break;
    }
}

void AirFryer::implMalfunction(std::shared_ptr<AirFryerData> data) {
    if (data == nullptr || !m_on)
        return;

    auto mf_id = getMfId(data);
    if (processCommonMf(mf_id))
        return;

    switch (mf_id) {
        using enum DemoDeviceData::MfId;
    case eHacked:
        // replace "Air" with "Evil"
        hackName("Evil", 3);
        break;
    case eBroken:
        addMalfunctionLog(std::format("{} is buring! BOOM! Buy a new one!\n", getName()), mf_id);
        m_on = false;
        break;
    default:
        // shouldn't reach here, because 2 common mf will be handled by processCommonMf
        throw Debug::DeviceOperationException<DeviceDataBase::MfId>(mf_id);
        break;
    }
}

uint32_t AirFryer::implTimeTravel(const uint32_t duration_sec) {
    // cook is sync (blocking sleep_for), thus no updated needed also
    std::this_thread::sleep_for(std::chrono::seconds(duration_sec));
    return duration_sec;
}

void AirFryer::cook(std::shared_ptr<AirFryerData> data) {
    auto op_id = AirFryerData::OpId::eAirFryerCook;
    // caller Operate() should filter out nullptr input
    Debug::logAssert(data != nullptr, "caller Operate() should filter out nullptr input");
    Debug::logAssert(
        data->food_volume > 0.f, "Food Volume shoud be positive, got %.3f", data->food_volume
    );

    if (data->food_volume > k_total_volume) {
        addOperationLog(
            std::format(
                "Food volume {} bigger than total volume {}. Buy a bigger one!",
                data->food_volume,
                k_total_volume
            ),
            op_id
        );
        data->success = false;
        return;
    } else if (data->food_volume > m_volume) {
        addOperationLog(
            std::format(
                "Food volume {} bigger than current volume {}. Wait for more space.",
                data->food_volume,
                m_volume
            ),
            op_id
        );
        data->success = false;
        return;
    }
    // else
    addOperationLog("Cooking starts.", op_id);
    m_volume -= data->food_volume;
    std::this_thread::sleep_for(std::chrono::seconds(data->time_sec));
    addOperationLog(std::format("Complete cooking after {} seconds.", data->time_sec), op_id);
    data->success = true;
}

void AirFryer::cleanup(std::shared_ptr<AirFryerData> data) {
    m_volume = k_total_volume;
    data->success = true;
    addOperationLog("Cleanup done.", AirFryerData::OpId::eAirFryerClean);
}
