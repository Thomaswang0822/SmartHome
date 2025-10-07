#include "washer_dryer.hpp"
#include <thread>
#include <utils.hpp>

void WasherDryer::operate(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->op_id) {
    case DeviceOpId::eWashDryerCombo:
        // auto call Dry() after Wash() finishes
        wash(data);
        break;
    case DeviceOpId::eWashDryerWashOnly:
        wash(data);
        break;
    case DeviceOpId::eWashDryerDryOnly:
        dry(data);
        break;
    default:
        Device::operate();
        data->logOpId();
        break;
    }
}

void WasherDryer::malfunction(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->mf_id) {
    case DeviceMfId::eLowBattery:
        m_on = false;
        break;
    case DeviceMfId::eHacked: {
        // replace "WasherDryer" with LuffyAce?
        std::string hack_name = data->dstring == "" ? "LuffyAce" : data->dstring;
        ::WasherDryer::hackName(hack_name, 11);
        break;
    }
    case DeviceMfId::eBroken:
        std::cout << getName() << " are leaking! See if Super Mario can help!" << std::endl;
        break;
    default:
        // eNormal
        Device::malfunction();
        break;
    }
}

void WasherDryer::finishAll() {
    while (!m_wash_bin.empty()) {
        performNext(true);
    }

    while (!m_dry_bin.empty()) {
        performNext(false);
    }
}

void WasherDryer::wash(std::shared_ptr<DeviceData> data) {
    Debug::logAssert(data != nullptr, "caller Operate() should filter out nullptr input");

    if (data->dfloat > k_total_volume) {
        data->dstring = std::format(
            "You put too much cloth ({}) more than total volume {}.\n", data->dfloat, k_total_volume
        );

        data->success = false;
        return;
    }

    // To simplify cases, all jobs should go thru the bin
    m_wash_bin.push_back(data);

    if (m_wash_timer.running) {
        // Every non-0th-submission goes here
        performNext(true /* is_wash */);
    }
    // before submitting the next wash
    Debug::logAssert(!m_wash_bin.empty(), "m_wash_bin should not be empty");
    Debug::logAssert(!m_wash_timer.running, "m_wash_timer should not be running");
    m_wash_timer.begin(data->dint);
}

void WasherDryer::dry(std::shared_ptr<DeviceData> data) {
    Debug::logAssert(data != nullptr, "caller Operate() should filter out nullptr input");

    if (data->dfloat > k_total_volume) {
        data->dstring = std::format(
            "You put too much cloth ({}) more than total volume {}.\n", data->dfloat, k_total_volume
        );

        data->success = false;
        return;
    }

    // To simplify cases, all jobs should go thru the bin
    m_dry_bin.push_back(data);

    if (m_dry_timer.running) {
        // Every non-0th-submission goes here
        performNext(false /* is_wash */);
    }
    // before submitting the next dry
    Debug::logAssert(!m_dry_bin.empty(), "m_dry_bin should not be empty");
    Debug::logAssert(!m_dry_timer.running, "m_dry_timer should not be running");
    m_dry_timer.begin(data->dint);
}

void WasherDryer::performNext(bool is_wash) {
    auto& timer = is_wash ? m_wash_timer : m_dry_timer;
    auto& bin = is_wash ? m_wash_bin : m_dry_bin;

    if (int remaining_time = timer.checkRemainingTime(); remaining_time > 0) {
        // sim till the end of previous job first
        std::this_thread::sleep_for(std::chrono::seconds(remaining_time));
        timer.stop();
    }
    // mark success and pop from bin
    auto prev_data = bin.front();
    bin.pop_front();
    prev_data->success = true;
    prev_data->dstring += std::format(
        "{} job completes after {} seconds. ", is_wash ? "Wash" : "Dry", prev_data->dint
    );

    // Check if this is a wash job in a wash-dry combo
    if (is_wash && prev_data->op_id == DeviceOpId::eWashDryerCombo) {
        // wash success but not combo
        prev_data->success = false;
        // submit to dryer.
        prev_data->dstring +=
            std::format("Begin dry in the combo, also take {} seconds; ", prev_data->dint);
        dry(prev_data);
    }
}
