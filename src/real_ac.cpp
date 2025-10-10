#include "real_ac.hpp"
#include "utils.hpp"

void RealAC::operate(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    // RealAc will modify Room
    Debug::logAssert(Device::s_room != nullptr, "Haven't logged room into {}", getName());
    // Check fields of `data` should happen in private worker functions.

    switch (data->op_id) {
    case DeviceOpId::eRealAcOpenTillDeg:
        openTillDeg(data);
        break;
    case DeviceOpId::eRealAcOpenForMins:
        openForMins(data);
        break;
    default:
        Device::operate();
        data->logOpId();
        break;
    }
}

void RealAC::malfunction(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->mf_id) {
    case DeviceMfId::eLowBattery:
        m_on = false;
        break;
    case DeviceMfId::eHacked: {
        std::cerr << getName() << " gets hacked! Burning everyone to death!" << std::endl;
        // stop curr op and burn to 45 deg
        updateTemp();
        m_timer.stop();
        data->dfloat = 45.f;
        data->dint = static_cast<int>(Mode::eFull);
        data->dbool = true; // set m_heat
        openTillDeg(data);
        break;
    }
    case DeviceMfId::eBroken:
        std::cerr << getName() << " is leaking! See if Super Mario can help!" << std::endl;
        break;
    default:
        // eNormal
        Device::malfunction();
        break;
    }
}

uint32_t RealAC::timeTravel(const uint32_t duration_sec) {
    uint32_t remaining_time =
        duration_sec == 0 ? static_cast<uint32_t>(m_timer.checkRemainingTime()) : duration_sec;
    std::this_thread::sleep_for(std::chrono::seconds(duration_sec));
    updateTemp();
    return remaining_time;
}

void RealAC::openTillDeg(std::shared_ptr<DeviceData> data) {
    // Step 0, store log
    std::string log_str = std::format(
        "openTillDeg() starts from {} at {}, set to {} on {}, targeting {} Celius degree",
        s_room->getTemp(),
        getCurrentTime(),
        data->dbool ? "heat" : "cool",
        data->dstring, // mode
        data->dfloat
    );
    // Step 1, validate input; dfloat, dbool, dstring are target temp, heat or not, mode
    Debug::logAssert(data != nullptr, "caller Operate() should filter out nullptr input");
    Debug::logAssert(
        (data->dfloat - s_room->getTemp()) > 0 == data->dbool, // heat/cool matches target
        "RealAC::openTillDeg(), current temperature is {}, but you set {} target temp {}",
        s_room->getTemp(),
        data->dbool ? "heat" : "cool",
        data->dfloat
    );

    // Step 2, finish previous AC session.
    updateTemp();
    if (m_timer.running)
        m_timer.stop();

    // Step 3, mode must be updated after updateTemp()
    bool set_mode_success = setMode(data->dstring);
    Debug::logAssert(set_mode_success, "RealAC::setMode() failed");
    // and update log
    data->dstring = log_str;

    // Step 4, set heat/cold, compute time, and launch new AC session
    m_heat = data->dbool;
    // time = delta temp / (power * K_DEG_PER_JOULE)
    float delta_temp = std::abs(s_room->getTemp() - data->dfloat);
    auto duration = static_cast<uint32_t>(delta_temp / (K_DEG_PER_JOULE * getPower()));
    m_timer.begin(duration);
}

void RealAC::openForMins(std::shared_ptr<DeviceData> data) {
    // Step 0, store log
    std::string log_str = std::format(
        "openForMins() starts from {} at {}, set to {} on {}, should run for {} sec",
        s_room->getTemp(),
        getCurrentTime(),
        data->dbool ? "heat" : "cool",
        data->dstring, // mode
        data->dint
    );

    // Step 1, validate input; dint, dbool, dstring are duration, heat or not, mode
    Debug::logAssert(data != nullptr, "caller Operate() should filter out nullptr input");
    Debug::logAssert(
        data->dint > 0,
        "RealAC::openForMins(), duration minutes should be positive, got {}",
        data->dint
    );

    // Step 2, finish previous AC session.
    updateTemp();
    if (m_timer.running)
        m_timer.stop();

    // Step 3, mode must be updated after updateTemp()
    bool set_mode_success = setMode(data->dstring);
    Debug::logAssert(set_mode_success, "RealAC::setMode() failed");
    // and update log
    data->dstring = log_str;

    // Step 4, set heat/cool and launch new AC session
    m_heat = data->dbool;
    m_timer.begin(data->dint);
}

void RealAC::updateTemp() {
    if (!m_timer.running)
        return;

    // This is actual execution time, with each sec simulation 1 min set by user.
    int op_time_sec = m_timer.t_total_sec.count() - m_timer.checkRemainingTime();
    float new_temp =
        s_room->getTemp() + (m_heat ? 1.0f : -1.0f) * K_DEG_PER_JOULE * getPower() * op_time_sec;
    s_room->setTemp(new_temp);
}

bool RealAC::setMode(std::string str) {
    // std::optional
    auto op_mode = magic_enum::enum_cast<Mode>(str);
    if (!op_mode.has_value()) {
        std::cerr << std::format(
            "{} is NOT a AC power mode. Supported are eFull, eMid, and eLow.\n", str
        );
        return false;
    }
    m_mode = op_mode.value();
    return true;
}
