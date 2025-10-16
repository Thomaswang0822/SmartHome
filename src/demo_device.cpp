// Placeholder content

#include "demo_device.hpp"

#include <chrono>
#include <format>
#include <iostream>

void DemoDevice::implOperate(std::shared_ptr<DeviceDataBase> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->op_id) {
    case OpId::eHello:
        data->log_str = hello();
        break;
    case OpId::eSing:
        data->log_str = sing();
        break;
    default:
        // OpId::eDefault
        Device::operate();
        break;
    }
    return;
}

void DemoDevice::implMalfunction(std::shared_ptr<DeviceDataBase> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->mf_id) {
    case DeviceDataBase::MfId::eLowBattery:
        m_on = false;
        break;
    case DeviceDataBase::MfId::eHacked:
        // replace "Demo" with "Evil"
        hackName("Evil", 4);
        break;
    case DeviceDataBase::MfId::eBroken:
        std::cerr << getName() << " is broken!" << std::endl;
        break;
    default:
        // eNormal
        Device::malfunction();
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
