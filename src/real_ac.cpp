#include "real_ac.hpp"
#include "utils.hpp"

void RealAC::implOperate(std::shared_ptr<RealACData> data) {
    if (data == nullptr || !m_on)
        return;

    // RealAc will modify Room
    Debug::logAssert(Device::s_room != nullptr, "Haven't logged room into {}", getName());
    // Check fields of `data` should happen in private worker functions.

    auto op_id = getOpId(data);
    switch (op_id) {
        using enum RealACData::OpId;
    case eRealAcOpenTillDeg:
        openTillDeg(data);
        break;
    case eRealAcOpenForMins:
        openForMins(data);
        break;
    default:
        throw Debug::DeviceOperationException<RealACData::OpId>(op_id);
        break;
    }
}

void RealAC::implMalfunction(std::shared_ptr<RealACData> data) {
    if (data == nullptr || !m_on)
        return;

    auto mf_id = getMfId(data);
    if (processCommonMf(mf_id))
        return;

    switch (mf_id) {
        using enum DemoDeviceData::MfId;
    case eHacked: {
        addMalfunctionLog(
            std::format("{} gets hacked! Burning everyone to death!", getName()), mf_id
        );
        // stop curr op and burn to 50 deg
        updateTemp();
        m_timer.stop();
        data->target_temp = 50.f;
        data->mode = RealACData::Mode::eFull;
        data->is_heat = true;
        openTillDeg(data);
        break;
    }
    case eBroken:
        addMalfunctionLog(
            std::format("{} is leaking! See if Super Mario can help!", getName()), mf_id
        );
        break;
    default:
        // shouldn't reach here
        throw Debug::DeviceOperationException<DeviceDataBase::MfId>(mf_id);
        break;
    }
}

uint32_t RealAC::implTimeTravel(const uint32_t duration_sec) {
    uint32_t remaining_time = static_cast<uint32_t>(m_timer.checkRemainingTime());
    uint32_t travel_time = duration_sec == 0 ? remaining_time : duration_sec;
    std::this_thread::sleep_for(std::chrono::seconds(travel_time));
    updateTemp();
    addMiscLog(
        std::format(
            "{} sec elapsed. Now temperature is {} degree.\n", remaining_time, s_room->getTemp()
        ),
        "TimeTravel"
    );
    return remaining_time;
}

void RealAC::openTillDeg(std::shared_ptr<RealACData> data) {
    // Step 1, validate input; dfloat, dbool, dstring are target temp, heat or not, mode
    Debug::logAssert(data != nullptr, "caller Operate() should filter out nullptr input");
    Debug::logAssert(
        (data->target_temp - s_room->getTemp()) > 0 == data->is_heat, // heat/cool matches target
        "RealAC::openTillDeg(), current temperature is {}, but you set {} target temp {}",
        s_room->getTemp(),
        data->is_heat ? "heat" : "cool",
        data->target_temp
    );

    // Step 2, finish previous AC session.
    updateTemp();
    if (m_timer.running)
        m_timer.stop();

    // Step 3, data must be updated after updateTemp()
    setMode(data->mode);
    // time = delta temp / (power * K_DEG_PER_JOULE)
    float delta_temp = std::abs(s_room->getTemp() - data->target_temp);
    data->duration_sec = std::round(delta_temp / (K_DEG_PER_JOULE * getPower(data->mode)));
    std::string log_str = std::format(
        "openForMins() starts from {} deg at {}, set to {} on {}, will run for {} sec and reach {} "
        "deg, but may be stopped early.",
        s_room->getTemp(),
        getCurrentTime(),
        data->is_heat ? "heat" : "cool",
        magic_enum::enum_name(data->mode),
        data->duration_sec,
        data->target_temp
    );
    addOperationLog(std::move(log_str), RealACData::OpId::eRealAcOpenTillDeg);

    // Step 4, Step 4, set heat/cool and launch new AC session
    m_heat = data->is_heat;
    m_timer.begin(data->duration_sec);
}

void RealAC::openForMins(std::shared_ptr<RealACData> data) {
    // Step 1, validate input; dint, dbool, dstring are duration, heat or not, mode
    Debug::logAssert(data != nullptr, "caller Operate() should filter out nullptr input");
    Debug::logAssert(
        data->duration_sec > 0,
        "RealAC::openForMins(), duration minutes should be positive, got {}",
        data->duration_sec
    );

    // Step 2, finish previous AC session.
    updateTemp();
    if (m_timer.running)
        m_timer.stop();

    // Step 3, data must be updated after updateTemp()
    setMode(data->mode);
    data->target_temp = s_room->getTemp() + (data->is_heat ? 1.0f : -1.0f) * K_DEG_PER_JOULE *
                                                getPower(m_mode) * data->duration_sec;
    std::string log_str = std::format(
        "openForMins() starts from {} deg at {}, set to {} on {}, will run for {} sec and reach {} "
        "deg, but may be stopped early.",
        s_room->getTemp(),
        getCurrentTime(),
        data->is_heat ? "heat" : "cool",
        magic_enum::enum_name(data->mode),
        data->duration_sec,
        data->target_temp
    );
    addOperationLog(std::move(log_str), RealACData::OpId::eRealAcOpenForMins);

    // Step 4, set heat/cool and launch new AC session
    m_heat = data->is_heat;
    m_timer.begin(data->duration_sec);
}

void RealAC::updateTemp() {
    if (!m_timer.running)
        return;

    // This is actual execution time, with each sec simulation 1 min set by user.
    int op_time_sec = m_timer.t_total_sec.count() - m_timer.checkRemainingTime();
    float new_temp = s_room->getTemp() +
                     (m_heat ? 1.0f : -1.0f) * K_DEG_PER_JOULE * getPower(m_mode) * op_time_sec;
    s_room->setTemp(new_temp);
}
