#include "washer_dryer.hpp"
#include <thread>
#include <utils.hpp>

void WasherDryer::Operate(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->op_id) {
    case DeviceOpId::eWashDryerCombo:
        // auto call Dry() after Wash() finishes
        Wash(data);
        break;
    case DeviceOpId::eWashDryerWashOnly:
        Wash(data);
        break;
    case DeviceOpId::eWashDryerDryOnly:
        Dry(data);
        break;
    default:
        Device::Operate();
        data->LogOpId();
        break;
    }
}

void WasherDryer::Malfunction(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->mf_id) {
    case DeviceMfId::eLowBattery:
        m_on = false;
        break;
    case DeviceMfId::eHacked: {
        // replace "WasherDryer" with LuffyAce?
        std::string hackName = data->dstring == "" ? "LuffyAce" : data->dstring;
        HackName(hackName, 11);
        break;
    }
    case DeviceMfId::eBroken:
        std::cout << GetName() << " are leaking! See if Super Mario can help!" << std::endl;
        break;
    default:
        // eNormal
        Device::Malfunction();
        break;
    }
}

void WasherDryer::FinishAll() {
    while (!m_wash_bin.empty()) {
        PerformNext(true);
    }

    while (!m_dry_bin.empty()) {
        PerformNext(false);
    }
}

void WasherDryer::Wash(std::shared_ptr<DeviceData> data) {
    Debug::Assert(data != nullptr, "caller Operate() should filter out nullptr input");

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
        PerformNext(true /* is_wash */);
    }
    // before submitting the next wash
    Debug::Assert(!m_wash_bin.empty(), "m_wash_bin should not be empty");
    Debug::Assert(!m_wash_timer.running, "m_wash_timer should not be running");
    m_wash_timer.Begin(data->dint);
}

void WasherDryer::Dry(std::shared_ptr<DeviceData> data) {
    Debug::Assert(data != nullptr, "caller Operate() should filter out nullptr input");

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
        PerformNext(false /* is_wash */);
    }
    // before submitting the next dry
    Debug::Assert(!m_dry_bin.empty(), "m_dry_bin should not be empty");
    Debug::Assert(!m_dry_timer.running, "m_dry_timer should not be running");
    m_dry_timer.Begin(data->dint);
}

void WasherDryer::PerformNext(bool is_wash) {
    auto& timer = is_wash ? m_wash_timer : m_dry_timer;
    auto& bin = is_wash ? m_wash_bin : m_dry_bin;

    if (int remaining_time = timer.CheckRemainingTime(); remaining_time > 0) {
        // sim till the end of previous job first
        std::this_thread::sleep_for(std::chrono::seconds(remaining_time));
        timer.Stop();
    }
    // mark success and pop from bin
    auto prevData = bin.front();
    bin.pop_front();
    prevData->success = true;
    prevData->dstring += std::format(
        "{} job completes after {} seconds. ", is_wash ? "Wash" : "Dry", prevData->dint
    );

    // Check if this is a wash job in a wash-dry combo
    if (is_wash && prevData->op_id == DeviceOpId::eWashDryerCombo) {
        // wash success but not combo
        prevData->success = false;
        // submit to dryer.
        prevData->dstring +=
            std::format("Begin dry in the combo, also take {} seconds; ", prevData->dint);
        this->Dry(prevData);
    }
}
