#include "real_ac.hpp"
#include "utils.hpp"

void RealAC::implOperate(std::shared_ptr<RealACData> data) {
    if (data == nullptr || !m_on)
        return;

    // RealAc will modify Room
    Debug::logAssert(Device::s_room != nullptr, "Haven't logged room into {}", getName());
    // Check fields of `data` should happen in private worker functions.

    switch (data->op_id) {
    case OpId::eRealAcOpenTillDeg:
        openTillDeg(data);
        break;
    case OpId::eRealAcOpenForMins:
        openForMins(data);
        break;
    default:
        Device::operate();
        data->logOpId();
        break;
    }
}

void RealAC::implMalfunction(std::shared_ptr<RealACData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->mf_id) {
    case RealACData::MfId::eLowBattery:
        m_on = false;
        break;
    case RealACData::MfId::eHacked: {
        std::cerr << getName() << " gets hacked! Burning everyone to death!" << std::endl;
        // stop curr op and burn to 45 deg
        updateTemp();
        m_timer.stop();
        data->target_temp = 45.f;
        data->duration_sec = static_cast<int>(RealACData::Mode::eFull);
        data->is_heat = true;
        openTillDeg(data);
        break;
    }
    case RealACData::MfId::eBroken:
        std::cerr << getName() << " is leaking! See if Super Mario can help!" << std::endl;
        break;
    default:
        // eNormal
        Device::malfunction();
        break;
    }
}

uint32_t RealAC::implTimeTravel(const uint32_t duration_sec) {
    uint32_t remaining_time =
        duration_sec == 0 ? static_cast<uint32_t>(m_timer.checkRemainingTime()) : duration_sec;
    std::this_thread::sleep_for(std::chrono::seconds(duration_sec));
    updateTemp();
    return remaining_time;
}

void RealAC::openTillDeg(std::shared_ptr<RealACData> data) {
    // Step 0, store log
    std::string log_str = std::format(
        "openTillDeg() starts from {} at {}, set to {} on {}, targeting {} Celius degree",
        s_room->getTemp(),
        getCurrentTime(),
        data->is_heat ? "heat" : "cool",
        magic_enum::enum_name(data->mode),
        data->target_temp
    );
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

    // Step 3, mode must be updated after updateTemp()
    setMode(data->mode);
    // and update log
    data->log_str = log_str;

    // Step 4, set heat/cold, compute time, and launch new AC session
    m_heat = data->is_heat;
    // time = delta temp / (power * K_DEG_PER_JOULE)
    float delta_temp = std::abs(s_room->getTemp() - data->target_temp);
    auto duration = static_cast<uint32_t>(delta_temp / (K_DEG_PER_JOULE * getPower(data->mode)));
    m_timer.begin(duration);
}

void RealAC::openForMins(std::shared_ptr<RealACData> data) {
    // Step 0, store log
    std::string log_str = std::format(
        "openForMins() starts from {} at {}, set to {} on {}, should run for {} sec",
        s_room->getTemp(),
        getCurrentTime(),
        data->is_heat ? "heat" : "cool",
        magic_enum::enum_name(data->mode),
        data->duration_sec
    );

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

    // Step 3, mode must be updated after updateTemp()
    setMode(data->mode);
    // and update log
    data->log_str = log_str;

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
