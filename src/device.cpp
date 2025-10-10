// Placeholder content

#include "device.hpp"

#include <chrono>
#include <format>
#include <iostream>

void Device::hackName(std::string newName, size_t len) {
    // Hack the name from the beginning
    m_name.replace(0, len, newName);
    std::cerr << "I got hacked and become " << getName() << std::endl;
}

void DemoDevice::operate(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->op_id) {
    case DeviceOpId::eHello:
        data->dstring = hello();
        break;
    case DeviceOpId::eSing:
        data->dstring = sing();
        break;
    default:
        // DeviceOpId::eDefault
        Device::operate();
        break;
    }
    return;
}

void DemoDevice::malfunction(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->mf_id) {
    case DeviceMfId::eLowBattery:
        m_on = false;
        break;
    case DeviceMfId::eHacked:
        // replace "Demo" with "Evil"
        hackName("Evil", 4);
        break;
    case DeviceMfId::eBroken:
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
