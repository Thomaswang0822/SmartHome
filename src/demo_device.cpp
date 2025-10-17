// Placeholder content

#include "demo_device.hpp"

#include <chrono>
#include <format>
#include <iostream>

void DemoDevice::implOperate(std::shared_ptr<DemoDeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    auto op_id = getOpId(data);
    switch (op_id) {
        using enum DemoDeviceData::OpId;
    case eHello:
        addOperationLog(hello(), op_id);
        break;
    case eSing:
        addOperationLog(sing(), op_id);
        break;
    default:
        // shouldn't reach here
        throw Debug::DeviceOperationException<DemoDeviceData::OpId>(op_id);
        break;
    }
    return;
}

void DemoDevice::implMalfunction(std::shared_ptr<DemoDeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    auto mf_id = getMfId(data);
    if (processCommonMf(mf_id))
        return;

    switch (mf_id) {
        using enum DemoDeviceData::MfId;
    case eHacked:
        // replace "Demo" with "Evil"
        hackName("Evil", 4);
        break;
    case eBroken:
        addMalfunctionLog(std::format("{} is broken!", getName()), mf_id);
        break;
    default:
        // shouldn't reach here
        throw Debug::DeviceOperationException<DeviceDataBase::MfId>(mf_id);
        break;
    }
}

std::string DemoDevice::hello() const {
    // std::format doesn't accept high_resolution_clock as it's not intended for display
    return std::format(
        "Hello World! This is device {}, greeting at {}!", getName(), getCurrentTime()
    );
}

std::string DemoDevice::sing() const { return "哈吉米, 哈吉米, 哈吉米, 哈吉米南北绿豆！"; }
