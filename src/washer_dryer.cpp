#include "washer_dryer.hpp"
#include <thread>
#include <utils.hpp>

void WasherDryer::implOperate(std::shared_ptr<WasherDryerData> data) {
    if (data == nullptr || !m_on)
        return;

    auto op_id = getOpId(data);
    switch (op_id) {
        using enum WasherDryerData::OpId;
    case eWashDryerCombo:
        // auto call Dry() after Wash() finishes
        wash(data);
        break;
    case eWashDryerWashOnly:
        wash(data);
        break;
    case eWashDryerDryOnly:
        dry(data);
        break;
    default:
        // shouldn't reach here
        throw Debug::DeviceOperationException<WasherDryerData::OpId>(op_id);
        break;
    }
}

void WasherDryer::implMalfunction(std::shared_ptr<WasherDryerData> data) {
    if (data == nullptr || !m_on)
        return;

    auto mf_id = getMfId(data);
    if (processCommonMf(mf_id))
        return;

    switch (mf_id) {
        using enum DemoDeviceData::MfId;
    case eHacked: {
        // replace "WasherDryer" with LuffyAce?
        std::string hack_name = data->hack_name == "" ? "LuffyAce" : data->hack_name;
        hackName(hack_name, 11);
        break;
    }
    case eBroken:
        addMalfunctionLog(
            std::format("{} are leaking! See if Super Mario can help!", getName()), mf_id
        );
        break;
    default:
        // shouldn't reach here
        throw Debug::DeviceOperationException<DeviceDataBase::MfId>(mf_id);
        break;
    }
}

uint32_t WasherDryer::implTimeTravel(const uint32_t duration_sec) {
    using namespace std::chrono;
    auto sim_remaining = seconds(duration_sec);
    auto constexpr ZERO_SEC = seconds(0);
    /// Used when duration_sec > 0
    auto canStep = [this, sim_remaining](bool is_wash) -> bool {
        auto& timer = is_wash ? m_wash_timer : m_dry_timer;
        auto& bin = is_wash ? m_wash_bin : m_dry_bin;
        if (!timer.running || bin.empty())
            return false;

        auto timeSpent = duration_cast<seconds>(system_clock::now() - timer.t_start);
        if (timeSpent > timer.t_total_sec) {
            // It finishes even before timeTravel simulation.
            return true;
        }
        auto time2Finish = timer.t_total_sec - timeSpent;
        return (time2Finish <= sim_remaining);
    };

    if (duration_sec == 0) {
        auto start = system_clock::now();
        // finish 1 wash and 1 dry if we should
        if (!m_wash_bin.empty() && m_wash_timer.running)
            performNext(true);
        if (!m_dry_bin.empty() && m_dry_timer.running)
            performNext(false);

        return duration_cast<seconds>(system_clock::now() - start).count();
    } else {
        bool wash_flag = true; // alternate
        while (sim_remaining > ZERO_SEC) {
            if (canStep(wash_flag)) {
                auto start = system_clock::now();
                performNext(wash_flag);
                sim_remaining -= duration_cast<seconds>(system_clock::now() - start);
            } else {
                /// Since we start with wash, we must check dry when no more wash can be done.
                /// But if no more dry can be done, we can break
                if (!wash_flag)
                    break;
            }
            wash_flag = !wash_flag;
        }

        std::this_thread::sleep_for(sim_remaining);
        return duration_sec;
    }
}

void WasherDryer::wash(std::shared_ptr<WasherDryerData> data) {
    auto op_id = getOpId(data);
    Debug::logAssert(data != nullptr, "caller Operate() should filter out nullptr input");

    if (data->cloth_volume > k_total_volume) {
        addOperationLog(
            std::format(
                "You put too much cloth ({}) more than total volume {}.\n",
                data->cloth_volume,
                k_total_volume
            ),
            op_id
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
    m_wash_timer.begin(data->wash_sec);
    addOperationLog("Wash job starts.", op_id);
}

void WasherDryer::dry(std::shared_ptr<WasherDryerData> data) {
    auto op_id = getOpId(data);
    Debug::logAssert(data != nullptr, "caller Operate() should filter out nullptr input");

    if (data->cloth_volume > k_total_volume) {
        addOperationLog(
            std::format(
                "You put too much cloth ({}) more than total volume {}.\n",
                data->cloth_volume,
                k_total_volume
            ),
            op_id
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
    m_dry_timer.begin(data->dry_sec);
    addOperationLog("Dry job starts.", op_id);
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
    auto op_id = getOpId(prev_data);
    bin.pop_front();
    prev_data->success = true;
    addOperationLog(
        std::format(
            "{} job completes after {} seconds, at {:%T}. ",
            is_wash ? "Wash" : "Dry",
            is_wash ? prev_data->wash_sec : prev_data->dry_sec,
            // not curr time, but time when job finished
            timer.t_start + timer.t_total_sec
        ),
        op_id
    );

    // Check if this is a wash job in a wash-dry combo
    if (is_wash && op_id == WasherDryerData::OpId::eWashDryerCombo) {
        // wash success but not combo
        prev_data->success = false;
        // submit to dryer.
        addOperationLog(
            std::format("Begin dry in the combo, also take {} seconds; ", prev_data->dry_sec), op_id
        );
        dry(prev_data);
    }
}
