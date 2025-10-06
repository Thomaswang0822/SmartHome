// Placeholder content

#include "device.hpp"

#include <chrono>
#include <format>
#include <iostream>

void Device::HackName(std::string newName, size_t len) {
    // Hack the name from the beginning
    m_name.replace(0, len, newName);
    std::cerr << "I got hacked and become " << GetName() << std::endl;
}

void DemoDevice::Operate(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->op_id) {
    case DeviceOpId::eHello:
        data->dstring = this->Hello();
        break;
    case DeviceOpId::eSing:
        data->dstring = this->Sing();
        break;
    default:
        // DeviceOpId::eDefault
        Device::Operate();
        break;
    }
    return;
}

void DemoDevice::Malfunction(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->mf_id) {
    case DeviceMfId::eLowBattery:
        m_on = false;
        break;
    case DeviceMfId::eHacked:
        // replace "Demo" with "Evil"
        HackName("Evil", 4);
        break;
    case DeviceMfId::eBroken:
        std::cerr << GetName() << " is broken!" << std::endl;
        break;
    default:
        // eNormal
        Device::Malfunction();
        break;
    }
}

std::string DemoDevice::Hello() const {
    // std::format doesn't accept high_resolution_clock as it's not intended for display
    return std::format(
        "Hello World! This is device {}, greeting at {}!",
        GetName(),
        std::chrono::system_clock::now()
    );
}

std::string DemoDevice::Sing() const { return "哈吉米, 哈吉米, 哈吉米, 哈吉米南北绿豆！"; }
